#include "sirius/vulkan/sync/semaphore.hpp"
#include "sirius/vulkan/core/vulkan.hpp"


namespace acma::vk {
	result<void> semaphore::wait(sl::uint64_t value, sl::uint64_t timeout) const noexcept {
		VkSemaphoreWaitInfo semaphore_post_wait_info{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.flags = 0,
			.semaphoreCount = 1,
			.pSemaphores = &smart_handle.get(),
			.pValues = &value
		};
		__D2D_VULKAN_VERIFY(sl::invoke(vulkan_fns_ptr->vkWaitSemaphores, *logi_device_ptr, &semaphore_post_wait_info, timeout));
		return {};
	}
}


namespace acma::impl {
	result<vk::semaphore>
		make<vk::semaphore>::
	operator()(
		sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
		sl::reference_ptr<const vk::logical_device> logi_device_ptr,
		VkSemaphoreType semaphore_type,
		sl::in_place_adl_tag_type<vk::semaphore>
	) const noexcept {
		vk::semaphore ret{};
		ret.smart_handle = {vulkan_fns_ptr->vkDestroySemaphore, logi_device_ptr};
		ret.vulkan_fns_ptr = vulkan_fns_ptr;
		ret.logi_device_ptr = logi_device_ptr;


        const VkSemaphoreTypeCreateInfo semaphore_type_info{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .pNext = nullptr,
            .semaphoreType = semaphore_type,
            .initialValue = 0,
        };

        const VkSemaphoreCreateInfo semaphore_info{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &semaphore_type_info,
        };

        __D2D_VULKAN_VERIFY(sl::invoke(vulkan_fns_ptr->vkCreateSemaphore, *logi_device_ptr, &semaphore_info, nullptr, &ret));
        return ret;
	}
}