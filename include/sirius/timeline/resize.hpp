#pragma once 
#include <streamline/numeric/int.hpp>

#include "sirius/core/window.hpp"
#include "sirius/timeline/command.fwd.hpp"
#include "sirius/timeline/state.hpp"
#include "sirius/vulkan/core/command_buffer.hpp"
#include "sirius/core/buffer_config_table.hpp"
#include "sirius/core/asset_heap_config_table.hpp"
#include "sirius/timeline/event.hpp"


namespace acma {
	template<buffer_key_t DstBufferKey, sl::size_t SizeBytes>
	struct resize : timeline::event {
		constexpr static command_family_t family = command_family::transfer;
	};

	template<buffer_key_t DstBufferKey, buffer_key_t SrcBufferKey>
	struct resize_to_fit : timeline::event {
		constexpr static command_family_t family = command_family::transfer;
	};
}

namespace acma::timeline {
	template<buffer_key_t DstBufferKey, sl::size_t SizeBytes>
	struct command<resize<DstBufferKey, SizeBytes>> {
		template<typename RenderProcessT, sl::index_t CommandGroupIdx>
		constexpr result<void> operator()(RenderProcessT& proc, window&, timeline::state&, sl::empty_t, sl::index_constant_type<CommandGroupIdx>) const noexcept {
			auto& dst_buff = sl::universal::get<DstBufferKey>(proc);
			if(SizeBytes <= dst_buff.capacity())
				return dst_buff.try_resize(SizeBytes);
			sl::invoke(proc.vulkan_functions_ptr()->vkDeviceWaitIdle, *proc.logical_device_ptr()); //TEMP
			return dst_buff.resize(SizeBytes);
		};
	};


	template<buffer_key_t DstBufferKey, buffer_key_t SrcBufferKey>
	struct command<resize_to_fit<DstBufferKey, SrcBufferKey>> {
		template<typename RenderProcessT, sl::index_t CommandGroupIdx>
		constexpr result<void> operator()(RenderProcessT& proc, window&, timeline::state&, sl::empty_t, sl::index_constant_type<CommandGroupIdx>) const noexcept {
			auto const& src_buff = sl::universal::get<SrcBufferKey>(proc);
			auto& dst_buff = sl::universal::get<DstBufferKey>(proc);
			const sl::size_t new_size = src_buff.size_bytes();
			if(new_size <= dst_buff.capacity())
				return dst_buff.try_resize(new_size);
			sl::invoke(proc.vulkan_functions_ptr()->vkDeviceWaitIdle, *proc.logical_device_ptr()); //TEMP
			return dst_buff.resize(new_size);
		};
	};
}