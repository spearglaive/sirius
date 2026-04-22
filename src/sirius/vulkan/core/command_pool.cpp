#include "sirius/vulkan/core/command_pool.hpp"
#include "sirius/core/command_family.hpp"
#include "sirius/vulkan/core/vulkan.hpp"


namespace acma::impl {
	result<vk::command_pool>
		make<vk::command_pool>::
	operator()(
		sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
		sl::reference_ptr<const vk::physical_device> phys_device_ptr,
		sl::reference_ptr<const vk::logical_device> logi_device_ptr,
		command_family_t family,
		sl::in_place_adl_tag_type<vk::command_pool>
	) const noexcept {
		vk::command_pool ret{{{vulkan_fns_ptr->vkDestroyCommandPool, logi_device_ptr}}};

        const VkCommandPoolCreateInfo command_pool_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = phys_device_ptr->queue_family_infos[family].index,
        };

        __D2D_VULKAN_VERIFY(sl::invoke(vulkan_fns_ptr->vkCreateCommandPool, *logi_device_ptr, &command_pool_info, nullptr, &ret));
        return ret;
	}
}