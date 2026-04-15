#pragma once

#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/api.def.h"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/core/vulkan_ptr.hpp"


__D2D_DECLARE_VK_TRAITS_DEVICE(VkImageView);

namespace acma::vk {
    struct SIRIUS_API image_view : vulkan_ptr<VkImageView, vkDestroyImageView> {
        // static result<image_view> create(std::shared_ptr<logical_device> device, image const& img, VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT) noexcept;
        static result<image_view> create(std::shared_ptr<logical_device> device, VkImageViewCreateInfo create_info) noexcept;
    };
}
