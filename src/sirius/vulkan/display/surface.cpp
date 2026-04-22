#include "sirius/vulkan/display/surface.hpp"


namespace acma::impl {
	result<vk::surface>
		make<vk::surface>::
	operator()(
		sl::reference_ptr<const vk::instance> instance_ptr,
		sl::reference_ptr<GLFWwindow> window_handle,
		sl::in_place_adl_tag_type<vk::surface> 
	) const noexcept {
		vk::surface ret{{{vkDestroySurfaceKHR, instance_ptr}}};
        __D2D_VULKAN_VERIFY(glfwCreateWindowSurface(*instance_ptr, window_handle.get(), nullptr, &ret));
        return ret;
	}
}