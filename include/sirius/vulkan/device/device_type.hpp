#pragma once
#include <cstdint>
#include <limits>
#include "sirius/vulkan/core/vulkan.hpp"


namespace acma::vk {
    enum class device_type : std::uint8_t {
        other,
        integrated_gpu,
        discrete_gpu,
        virtual_gpu,
        cpu,

        unknown = std::numeric_limits<std::uint8_t>::max(),
    };
}
