#include "sirius/vulkan/display/image_view.hpp"

namespace acma::vk {
    result<image_view> image_view::create(std::shared_ptr<logical_device> device, VkImageViewCreateInfo create_info) noexcept {
        image_view ret{};
        ret.dependent_handle = device;
        __D2D_VULKAN_VERIFY(vkCreateImageView(*device, &create_info, nullptr, &ret));
        return ret;
    }
}