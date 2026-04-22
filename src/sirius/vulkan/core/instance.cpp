#include "sirius/vulkan/core/instance.hpp"
#include "sirius/vulkan/core/vulkan.hpp"


namespace acma::impl {
	result<vk::instance>
		make<vk::instance>::
	operator()(
		VkInstanceCreateInfo create_info,
		sl::in_place_adl_tag_type<vk::instance>
	) const noexcept {
		vk::instance ret{{{vkDestroyInstance}}};
        __D2D_VULKAN_VERIFY(sl::invoke(vkCreateInstance, &create_info, nullptr, &ret));
        return ret;
	}
}