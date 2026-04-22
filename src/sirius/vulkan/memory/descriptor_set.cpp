#include "sirius/vulkan/memory/descriptor_set.hpp"
#include "sirius/vulkan/core/vulkan.hpp"



namespace acma::vk {
	void descriptor_set::update(std::span<const VkWriteDescriptorSet> writes) noexcept {
		return sl::invoke(vulkan_fns_ptr->vkUpdateDescriptorSets, *logi_device_ptr, writes.size(), writes.data(), 0, nullptr);
	}
}

namespace acma::impl {
	result<vk::descriptor_set>
		make<vk::descriptor_set>::
	operator()(
		sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
		sl::reference_ptr<const vk::logical_device> logi_device_ptr,
		//sl::reference_ptr<const vk::descriptor_pool> descriptor_pool_ptr,
		VkDescriptorSetAllocateInfo create_info,
		sl::in_place_adl_tag_type<vk::descriptor_set>
	) const noexcept {
		vk::descriptor_set ret{};
		//ret.smart_handle = {vulkan_fns_ptr->vkFreeDescriptorSets, logi_device_ptr, descriptor_pool_ptr};
		ret.vulkan_fns_ptr = vulkan_fns_ptr;
		ret.logi_device_ptr = logi_device_ptr;
        __D2D_VULKAN_VERIFY(sl::invoke(vulkan_fns_ptr->vkAllocateDescriptorSets, *logi_device_ptr, &create_info, &ret));
        return ret;
	}
}