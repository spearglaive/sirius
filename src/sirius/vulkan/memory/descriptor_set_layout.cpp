#include "sirius/vulkan/memory/descriptor_set_layout.hpp"
#include "sirius/vulkan/core/vulkan.hpp"


namespace acma::impl {
	result<vk::descriptor_set_layout>
		make<vk::descriptor_set_layout>::
	operator()(
		sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
		sl::reference_ptr<const vk::logical_device> logi_device_ptr,
		VkDescriptorSetLayoutCreateInfo create_info,
		sl::in_place_adl_tag_type<vk::descriptor_set_layout>
	) const noexcept {
		vk::descriptor_set_layout ret{{{vulkan_fns_ptr->vkDestroyDescriptorSetLayout, logi_device_ptr}}};
        __D2D_VULKAN_VERIFY(sl::invoke(vulkan_fns_ptr->vkCreateDescriptorSetLayout, *logi_device_ptr, &create_info, nullptr, &ret));
        return ret;
	}
}