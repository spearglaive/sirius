#pragma once
#include "sirius/arith/point.hpp"
#include "sirius/arith/size.hpp"
#include "sirius/core/buffer_config.hpp"
#include "sirius/core/buffer_key_t.hpp"
#include "sirius/core/error.hpp"
#include "sirius/vulkan/memory/buffer.fwd.hpp"
#include "sirius/vulkan/memory/buffer_allocation.hpp"
#include "sirius/vulkan/memory/asset_heap.fwd.hpp"
#include "sirius/vulkan/memory/image_allocation.hpp"


namespace acma::vk {
	struct SIRIUS_API image;
}


namespace acma {
	template<buffer_config DstConfig, buffer_config SrcConfig, typename RenderProcessT>
	constexpr result<void> gpu_copy(
		RenderProcessT& process,
		vk::buffer_allocation_unique_ptr& dst,
		sl::constant_type<buffer_config, DstConfig>,
		vk::buffer_allocation_unique_ptr const& src,
		sl::constant_type<buffer_config, SrcConfig>,
		sl::size_t size,
		sl::uoffset_t dst_offset = 0,
		sl::uoffset_t src_offset = 0
	) noexcept;

	template<buffer_key_t DstK, buffer_key_t SrcK, auto BufferConfigs, typename RenderProcessT>
	constexpr result<void> gpu_copy(
		vk::buffer<DstK, BufferConfigs, RenderProcessT>& dst,
		vk::buffer<SrcK, BufferConfigs, RenderProcessT> const& src,
		sl::size_t size,
		sl::uoffset_t dst_offset = 0,
		sl::uoffset_t src_offset = 0
	) noexcept;


	template<buffer_key_t DstK, buffer_key_t SrcK, auto BufferConfigs, typename RenderProcessT>
	constexpr result<void> gpu_copy(
		RenderProcessT& process,
		vk::image& dst,
		vk::image const& src,
		extent3 size,
		offset3 dst_offset = {},
		offset3 src_offset = {},
		sl::uint64_t timeout = std::numeric_limits<sl::uint64_t>::max()
	) noexcept;


	template<asset_heap_key_t DstK, buffer_key_t SrcK, auto AssetHeapConfigs, auto BufferConfigs, typename RenderProcessT>
	constexpr result<void> gpu_copy(
		vk::asset_heap<DstK, AssetHeapConfigs, RenderProcessT> & dst,
		vk::buffer<SrcK, BufferConfigs, RenderProcessT> const& src,
		sl::uint64_t timeout = std::numeric_limits<sl::uint64_t>::max()
	) noexcept;
}

namespace acma {
	template<buffer_config Config, typename RenderProcessT>
	constexpr result<vk::buffer_allocation_unique_ptr> gpu_allocate(
		RenderProcessT const& process,
		sl::constant_type<buffer_config, Config> = {}
	) noexcept;
	
	template<typename RenderProcessT>
	constexpr result<vk::buffer_allocation_unique_ptr> gpu_allocate_like(
		RenderProcessT const& process,
		vk::buffer_allocation_unique_ptr const& old_allocation,
		sl::size_t new_size_bytes
	) noexcept;


	constexpr result<vk::image_allocation_unique_ptr> gpu_allocate(
		vk::allocator_shared_handle const& allocator_ptr,
		vk::image_creation_info_t create_info,
		bool dedicated_allocation = false
	) noexcept;
}
