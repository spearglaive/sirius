#pragma once
#include "sirius/vulkan/core/vulkan.hpp"
#include "sirius/vulkan/core/vulkan.hpp"

namespace acma::vk {
    enum class present_mode {
        immediate    = VK_PRESENT_MODE_IMMEDIATE_KHR,
        mailbox      = VK_PRESENT_MODE_MAILBOX_KHR,
        fifo         = VK_PRESENT_MODE_FIFO_KHR,
        fifo_relaxed = VK_PRESENT_MODE_FIFO_RELAXED_KHR,

        num_present_modes
    };
}