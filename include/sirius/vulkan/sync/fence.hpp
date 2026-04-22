#pragma once
#include "sirius/vulkan/device/logical_device.hpp"
#include <cstdint>
#include "sirius/vulkan/core/vulkan.hpp"

namespace acma::vk {
    struct fence : vulkan_ptr<VkFence, vkDestroyFence> {
        static result<fence> create(sl::reference_ptr<const logical_device> device) noexcept;

    public:
        result<void> wait(std::uint64_t timeout = UINT64_MAX) const noexcept;
        result<void> reset() const noexcept;
    };
}