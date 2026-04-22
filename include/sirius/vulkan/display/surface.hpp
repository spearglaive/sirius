#pragma once
#include <memory>

#include "sirius/vulkan/core/vulkan.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "sirius/core/api.def.h"
#include "sirius/vulkan/core/mixin.hpp"
#include "sirius/vulkan/core/instance.hpp"

namespace acma::vk {
    struct SIRIUS_API surface : mixin<VkSurfaceKHR, PFN_vkDestroySurfaceKHR, instance> {};
}


namespace acma::impl {
	template<>
    struct make<vk::surface> {
		SIRIUS_API result<vk::surface> operator()(
			sl::reference_ptr<const vk::instance> instance_ptr,
			sl::reference_ptr<GLFWwindow> window_handle,
			sl::in_place_adl_tag_type<vk::surface> = sl::in_place_adl_tag<vk::surface>
		) const noexcept;
	};
}
