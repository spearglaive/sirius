#include "sirius/vulkan/display/image_view_handle.hpp"


namespace acma::impl {
	result<vk::image_view_handle>
		make<vk::image_view_handle>::
	operator()(
		sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
		sl::reference_ptr<const vk::logical_device> logi_device_ptr,
		vk::image_view associated_view,
		sl::in_place_adl_tag_type<vk::image_view_handle>
	) const noexcept {
		vk::image_view_handle ret{associated_view, {{vulkan_fns_ptr->vkDestroyImageView, logi_device_ptr}}};
		const VkImageViewCreateInfo create_info = static_cast<VkImageViewCreateInfo>(associated_view);
		__D2D_VULKAN_VERIFY(sl::invoke(vulkan_fns_ptr->vkCreateImageView, *logi_device_ptr, &create_info, nullptr, &ret));
		return ret;
	}
}
