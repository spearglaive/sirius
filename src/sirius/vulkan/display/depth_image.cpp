#include "sirius/vulkan/display/depth_image.hpp"

#include "sirius/vulkan/core/vulkan.hpp"
#include "sirius/core/make.hpp"



namespace acma::impl {
	result<vk::depth_image>
		make<vk::depth_image>::
	operator()(
		sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
		sl::reference_ptr<const vk::logical_device> logi_device_ptr,
		sl::reference_ptr<const vk::allocator> allocator,
		extent2 extent,
		sl::in_place_adl_tag_type<vk::depth_image>
	) const noexcept {
        vk::depth_image ret{};

		const VkImageCreateInfo image_create_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = VK_FORMAT_D32_SFLOAT,
            .extent = {extent.width(), extent.height(), 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        RESULT_TRY_MOVE(static_cast<vk::image&>(ret), ::acma::make<vk::image>(vulkan_fns_ptr, logi_device_ptr, allocator, image_create_info, true, VK_IMAGE_ASPECT_DEPTH_BIT));
		return ret;
	};
}