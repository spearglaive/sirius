#include "sirius/vulkan/display/depth_image.hpp"

#include "sirius/vulkan/core/vulkan.hpp"
#include "sirius/core/make.hpp"


namespace acma::vk {
    result<depth_image> depth_image::create(
		std::shared_ptr<logical_device> const& logi_device,
		allocator_shared_handle const& allocator,
		extent2 extent
	) noexcept {
        depth_image ret{};

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

        RESULT_TRY_MOVE(static_cast<image&>(ret), make<image>(logi_device, allocator, image_create_info, true, VK_IMAGE_ASPECT_DEPTH_BIT));

        return ret;
    }
}
