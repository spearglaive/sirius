#include "sirius/vulkan/core/instance.hpp"
#include "sirius/vulkan/core/vulkan.hpp"


namespace acma::impl {
	result<vk::instance>
		make<vk::instance>::
	operator()(
		VkInstanceCreateInfo create_info,
		sl::in_place_adl_tag_type<vk::instance>
	) const noexcept {
		VkInstance handle = VK_NULL_HANDLE;
        __D2D_VULKAN_VERIFY(sl::invoke(vkCreateInstance, &create_info, nullptr, &handle));
		volkLoadInstanceOnly(handle);

		return vk::instance{{{vkDestroyInstance, handle}}};
	}
}