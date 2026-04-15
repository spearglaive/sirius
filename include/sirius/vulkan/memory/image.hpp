#pragma once
#include <memory>

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
#include "sirius/vulkan/memory/buffer.fwd.hpp"
#include "sirius/core/memory_management.fwd.hpp"
#include "sirius/vulkan/memory/allocator.hpp"


// __D2D_DECLARE_VK_TRAITS_DEVICE(VkImage);

namespace acma::vk {
    struct SIRIUS_API image {
        static result<image> create(
			std::shared_ptr<logical_device> const& logi_device,
			allocator_shared_handle const& allocator,
			image_creation_info_t create_info,
			bool dedicated_allocation = false,
			VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT
		) noexcept;
        
    public:
		constexpr sl::size_t dimensions() const noexcept { return static_cast<sl::size_t>(alloc_ptr->creation_info.imageType) + 1; }

		constexpr VkImage const& handle() const& noexcept { return alloc_ptr->handle; }
		constexpr image_view const& view() const& noexcept { return associated_view; }

		constexpr VkImageCreateInfo     const& creation_info()   const& noexcept { return alloc_ptr->creation_info; }
        constexpr VkFormat              const& format_id()       const& noexcept { return alloc_ptr->creation_info.format; }
        constexpr VkExtent3D            const& size()            const& noexcept { return alloc_ptr->creation_info.extent; }
        constexpr sl::uint32_t          const& mip_level_count() const& noexcept { return alloc_ptr->creation_info.mipLevels; }
        constexpr sl::uint32_t          const& layer_count()     const& noexcept { return alloc_ptr->creation_info.arrayLayers; }
        constexpr VkSampleCountFlagBits const& sample_count()    const& noexcept { return alloc_ptr->creation_info.samples; }
        constexpr VkImageTiling         const& tiling()          const& noexcept { return alloc_ptr->creation_info.tiling; }
        constexpr VkImageUsageFlags     const& usage()           const& noexcept { return alloc_ptr->creation_info.usage; }
		
        constexpr sl::size_t            const& size_bytes()      const& noexcept { return alloc_ptr->allocation_info.size; }

        constexpr VkImageLayout const& layout() const& noexcept { return this->current_layout; }


    protected:
	    image_allocation_unique_ptr alloc_ptr;
		VkImageLayout current_layout;
		image_view associated_view;
    public:
		template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
		friend class asset_heap;
		
		template<asset_heap_key_t DstK, buffer_key_t SrcK, auto AssetHeapConfigs, auto BufferConfigs, typename RenderProcessT>
		friend constexpr result<void> acma::gpu_copy(
			vk::asset_heap<DstK, AssetHeapConfigs, RenderProcessT> & dst,
			vk::buffer<SrcK, BufferConfigs, RenderProcessT> const& src,
			sl::uint64_t timeout
		) noexcept;

		template<buffer_key_t DstK, buffer_key_t SrcK, auto BufferConfigs, typename RenderProcessT>
		friend constexpr result<void> acma::gpu_copy(
			RenderProcessT& proc,
			vk::image& dst,
			vk::image const& src,
			extent3 size,
			offset3 dst_offset,
			offset3 src_offset,
			sl::uint64_t timeout
		) noexcept;
    };
}
