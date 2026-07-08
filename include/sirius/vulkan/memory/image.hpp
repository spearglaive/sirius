#pragma once
#include <memory>

#include "sirius/core/make.hpp"
#include "sirius/vulkan/core/mixin.hpp"
#include "sirius/vulkan/core/unique_vk_ptr.hpp"
#include "sirius/arith/point.hpp"
#include "sirius/arith/size.hpp"
#include "sirius/core/asset_heap_key_t.hpp"
#include "sirius/core/buffer_key_t.hpp"
#include "sirius/core/error.hpp"
#include "sirius/vulkan/core/vulkan.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/display/image_view.hpp"
#include "sirius/vulkan/memory/image_allocation.hpp"
#include "sirius/vulkan/memory/asset_heap.fwd.hpp"
#include "sirius/vulkan/memory/resizable_gpu_buffer.fwd.hpp"
#include "sirius/core/memory_management.fwd.hpp"
#include "sirius/vulkan/memory/allocator.hpp"


// __D2D_DECLARE_VK_TRAITS_DEVICE(VkImage);

namespace acma::vk {
    struct SIRIUS_API image {
	public:
		template<typename T>
		friend struct ::acma::impl::make;

    public:
		constexpr sl::reference_ptr<const VkImage>    handle_ref() const& noexcept { return {std::addressof(alloc_ptr->handle)}; }
		constexpr sl::reference_ptr<const image_view> view_ref  () const& noexcept { return {std::addressof(associated_view)}; }

		constexpr sl::size_t dimensions() const noexcept { return static_cast<sl::size_t>(alloc_ptr->creation_info.imageType) + 1; }

		constexpr VkImageCreateInfo     creation_info()   const noexcept { return alloc_ptr->creation_info; }
        constexpr VkFormat              format_id()       const noexcept { return alloc_ptr->creation_info.format; }
        constexpr VkExtent3D            size()            const noexcept { return alloc_ptr->creation_info.extent; }
        constexpr sl::uint32_t          mip_level_count() const noexcept { return alloc_ptr->creation_info.mipLevels; }
        constexpr sl::uint32_t          layer_count()     const noexcept { return alloc_ptr->creation_info.arrayLayers; }
        constexpr VkSampleCountFlagBits sample_count()    const noexcept { return alloc_ptr->creation_info.samples; }
        constexpr VkImageTiling         tiling()          const noexcept { return alloc_ptr->creation_info.tiling; }
        constexpr VkImageUsageFlags     usage()           const noexcept { return alloc_ptr->creation_info.usage; }

        constexpr sl::size_t            size_bytes()      const noexcept { return alloc_ptr->allocation_info.size; }

        constexpr VkImageLayout layout() const noexcept { return this->current_layout; }


    protected:
	    image_allocation_unique_ptr alloc_ptr;
		VkImageLayout current_layout;
		image_view associated_view;
    public:
		template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
		friend class asset_heap;

		template<sl::size_t CommandGroupCount>
		friend constexpr result<void> acma::gpu_copy(
			render_process_core<CommandGroupCount> const& process_core,
			vk::image& dst,
			vk::image const& src,
			extent3 size,
			offset3 dst_offset,
			offset3 src_offset,
			sl::uint64_t timeout
		) noexcept;


		template<sl::size_t CommandGroupCount, sl::traits::specialization_of<vk::generic::resizable_gpu_buffer> SrcBufferT>
		friend constexpr result<void> acma::gpu_copy(
			render_process_core<CommandGroupCount> const& process_core,
			std::span<vk::image> dst,
			SrcBufferT const& src,
			sl::uint64_t timeout
		) noexcept;
    };
}


namespace acma::impl {
	template<>
    struct make<vk::image> {
		SIRIUS_API result<vk::image> operator()(
			sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
			sl::reference_ptr<const vk::logical_device> logi_device_ptr,
			sl::reference_ptr<const vk::allocator> allocator,
			vk::image_creation_info_t create_info,
			bool dedicated_allocation = false,
			VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT,
			sl::in_place_adl_tag_type<vk::image> = sl::in_place_adl_tag<vk::image>
		) const noexcept;
	};
}
