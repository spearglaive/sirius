#include "sirius/vulkan/memory/descriptor_pool.hpp"
#include "sirius/vulkan/core/vulkan.hpp"


namespace acma::impl {
	result<vk::descriptor_pool>
		make<vk::descriptor_pool>::
	operator()(
		sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
		sl::reference_ptr<const vk::logical_device> logi_device_ptr,
		VkDescriptorPoolCreateInfo create_info,
		sl::in_place_adl_tag_type<vk::descriptor_pool>
	) const noexcept {
		vk::descriptor_pool ret{{{vulkan_fns_ptr->vkDestroyDescriptorPool, logi_device_ptr}}};
        __D2D_VULKAN_VERIFY(sl::invoke(vulkan_fns_ptr->vkCreateDescriptorPool, *logi_device_ptr, &create_info, nullptr, &ret));
        return ret;
	}
}