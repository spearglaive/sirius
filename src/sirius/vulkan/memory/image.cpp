#include "sirius/vulkan/memory/image.hpp"

#include "sirius/core/make.hpp"
#include "sirius/core/memory_management.hpp"



namespace acma::impl {
	result<vk::image>
		make<vk::image>::
	operator()(
		sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
		sl::reference_ptr<const vk::logical_device> logi_device_ptr,
		sl::reference_ptr<const vk::allocator> allocator,
		vk::image_creation_info_t create_info,
		bool dedicated_allocation,
		VkImageAspectFlags aspect_mask,
		sl::in_place_adl_tag_type<vk::image>
	) const noexcept {
        vk::image ret{};
		RESULT_TRY_MOVE(ret.alloc_ptr, gpu_allocate(allocator, create_info, dedicated_allocation));
	
		const VkImageViewCreateInfo image_view_create_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = *ret.handle_ref(),
            .viewType = ret.layer_count() > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D,
            .format = ret.format_id(),
            .components{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
            .subresourceRange{
                .aspectMask = aspect_mask,
                .baseMipLevel = 0,
                .levelCount = ret.mip_level_count(),
                .baseArrayLayer = 0,
                .layerCount = ret.layer_count(),
            },
        };
		RESULT_TRY_MOVE(ret.associated_view, ::acma::make<vk::image_view>(vulkan_fns_ptr, logi_device_ptr, image_view_create_info));
		return ret;
	};
}