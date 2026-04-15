#pragma once

#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/api.def.h"


namespace acma::vk::impl {
	SIRIUS_API constexpr VkInstance& vulkan_instance() noexcept {
		static VkInstance i{};
		return i;
	}
}
