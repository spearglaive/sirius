#include "sirius/vulkan/memory/image.hpp"

#include "sirius/core/make.hpp"
#include "sirius/core/memory_management.hpp"


namespace acma::vk {
    result<image> image::create(
		std::shared_ptr<logical_device> const& logi_device,
		allocator_shared_handle const& allocator,
		image_creation_info_t create_info,
		bool dedicated_allocation,
		VkImageAspectFlags aspect_mask
	) noexcept {
        image ret{};
		RESULT_TRY_MOVE(ret.alloc_ptr, gpu_allocate(allocator, create_info, dedicated_allocation));
	
		const VkImageViewCreateInfo image_view_create_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = ret.handle(),
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
		RESULT_TRY_MOVE(ret.associated_view, make<image_view>(logi_device, image_view_create_info))
        return ret;
    }
}