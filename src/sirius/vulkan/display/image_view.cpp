#include "sirius/vulkan/display/image_view.hpp"


namespace acma::impl {
	result<vk::image_view>
		make<vk::image_view>::
	operator()(
		sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
		sl::reference_ptr<const vk::logical_device> logi_device_ptr,
		VkImageViewCreateInfo create_info,
		sl::in_place_adl_tag_type<vk::image_view>
	) const noexcept {
		vk::image_view ret{{{vulkan_fns_ptr->vkDestroyImageView, logi_device_ptr}}};
		__D2D_VULKAN_VERIFY(sl::invoke(vulkan_fns_ptr->vkCreateImageView, *logi_device_ptr, &create_info, nullptr, &ret))
		return ret;
	}
}