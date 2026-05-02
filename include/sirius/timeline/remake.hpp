#pragma once
#include <streamline/numeric/int.hpp>
#include <streamline/universal/get.hpp>

#include "sirius/core/buffer_config.hpp"
#include "sirius/core/buffer_key_t.hpp"
#include "sirius/core/error.hpp"
#include "sirius/core/memory_management.hpp"
#include "sirius/vulkan/memory/buffer_allocation.hpp"
#include "sirius/vulkan/core/command_buffer.hpp"


namespace acma::timeline::impl {
	template<
		buffer_key_t Key, bool PreserveOldAllocation, bool CopyData,
		typename RenderProcessT, sl::index_t CommandGroupIdx
	>
	constexpr result<void> remake(
		RenderProcessT& proc,
		sl::index_constant_type<CommandGroupIdx>,
		sl::size_t new_size
	) noexcept {
		auto& dst_buff = sl::universal::get<Key>(proc);

		if(new_size == 0) return {};

		if(new_size <= dst_buff.capacity()) {
			return dst_buff.try_resize(new_size);
			// return dst_buff.try_recreate(new_size); //TODO
		}
		else if (proc.frame_count() == 0) [[unlikely]] {
			return dst_buff.resize(new_size);
			// return dst_buff.recreate(new_size); //TODO
		}

		constexpr buffer_config config = sl::universal::get<Key>(proc).config;

		if constexpr(config.coupling != coupling_policy::coupled && !PreserveOldAllocation && !CopyData){
			RESULT_TRY_MOVE(dst_buff.allocation_ptr(), acma::gpu_allocate_like(proc, dst_buff.allocation_ptr(), new_size));
		} else {
			//If the buffer is gpu exclusive, it may still be in use by the gpu if the gpu is still on the previous frame
			//Thus, we cannot destroy the buffer until the beginning of the next frame, even if we don't care about preserving it
			//(If we do care about preserving it or need to copy the old data, we have to wait until the next frame with the same frame index anyway)
			constexpr bool can_destroy_old_alloc_next_frame = (config.coupling == coupling_policy::coupled && !PreserveOldAllocation && !CopyData);
			const sl::index_t frame_index = can_destroy_old_alloc_next_frame ? proc.next_frame_index() : proc.frame_index();

			std::vector<acma::vk::buffer_allocation_unique_ptr>& old_allocs = proc.old_allocations()[frame_index];

			old_allocs.push_back(std::move(dst_buff.allocation_ptr()));
			acma::vk::buffer_allocation_unique_ptr& old_alloc = old_allocs.back();
			RESULT_TRY_MOVE(dst_buff.allocation_ptr(), acma::gpu_allocate_like(proc, old_alloc, new_size));

			if constexpr(!CopyData) return {};

			vk::command_buffer const& transfer_command_buffer = proc.command_buffers()[proc.frame_index()][CommandGroupIdx];
			transfer_command_buffer.copy(dst_buff.allocation_ptr(), old_alloc, old_alloc->creation_info.size);
		}


		return {};
	}
}
