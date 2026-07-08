#pragma once
#include <streamline/metaprogramming/type_traits/relationships.hpp>

#include "sirius/arith/point.hpp"
#include "sirius/arith/size.hpp"
#include "sirius/core/buffer_config.hpp"
#include "sirius/core/buffer_key_t.hpp"
#include "sirius/core/error.hpp"
#include "sirius/core/render_process_core.fwd.hpp"
#include "sirius/vulkan/memory/buffer_allocation.hpp"
#include "sirius/vulkan/memory/asset_heap.fwd.hpp"
#include "sirius/vulkan/memory/image_allocation.hpp"
#include "sirius/vulkan/memory/resizable_gpu_buffer.fwd.hpp"


namespace acma::vk {
	struct SIRIUS_API image;
}


namespace acma {
	template<sl::size_t CommandGroupCount, buffer_config DstConfig, buffer_config SrcConfig>
	constexpr result<void> gpu_copy(
		render_process_core<CommandGroupCount>& process_core,
		vk::buffer_allocation_unique_ptr& dst,
		sl::constant_type<buffer_config, DstConfig>,
		vk::buffer_allocation_unique_ptr const& src,
		sl::constant_type<buffer_config, SrcConfig>,
		sl::size_t size,
		sl::uoffset_t dst_offset = 0,
		sl::uoffset_t src_offset = 0
	) noexcept;

	template<
		sl::size_t CommandGroupCount,
		sl::traits::specialization_of<vk::generic::resizable_gpu_buffer> DstBufferT,
		sl::traits::specialization_of<vk::generic::resizable_gpu_buffer> SrcBufferT
	>
	constexpr result<void> gpu_copy(
		render_process_core<CommandGroupCount>& process_core,
		DstBufferT& dst,
		SrcBufferT const& src,
		sl::size_t size,
		sl::uoffset_t dst_offset = 0,
		sl::uoffset_t src_offset = 0
	) noexcept;


	template<sl::size_t CommandGroupCount>
	constexpr result<void> gpu_copy(
		render_process_core<CommandGroupCount> const& process_core,
		vk::image& dst,
		vk::image const& src,
		extent3 size,
		offset3 dst_offset = {},
		offset3 src_offset = {},
		sl::uint64_t timeout = std::numeric_limits<sl::uint64_t>::max()
	) noexcept;


	template<sl::size_t CommandGroupCount, sl::traits::specialization_of<vk::generic::resizable_gpu_buffer> SrcBufferT>
	constexpr result<void> gpu_copy(
		render_process_core<CommandGroupCount> const& process_core,
		std::span<vk::image> dst,
		SrcBufferT const& src,
		sl::uint64_t timeout = std::numeric_limits<sl::uint64_t>::max()
	) noexcept;
}

namespace acma {
	template<buffer_config Config, sl::size_t CommandGroupCount>
	constexpr result<vk::buffer_allocation_unique_ptr> gpu_allocate(
		render_process_core<CommandGroupCount> const& process_core,
		sl::constant_type<buffer_config, Config> = {}
	) noexcept;

	template<sl::size_t CommandGroupCount>
	constexpr result<vk::buffer_allocation_unique_ptr> gpu_allocate_like(
		render_process_core<CommandGroupCount> const& process_core,
		vk::buffer_allocation_unique_ptr const& old_allocation,
		sl::size_t new_size_bytes
	) noexcept;


	constexpr result<vk::image_allocation_unique_ptr> gpu_allocate(
		sl::reference_ptr<const vk::allocator> allocator_ptr,
		vk::image_creation_info_t create_info,
		bool dedicated_allocation = false
	) noexcept;
}
