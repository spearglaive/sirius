#pragma once

#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/api.def.h"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/core/make.hpp"
#include "sirius/vulkan/core/unique_vk_ptr.hpp"
#include "sirius/vulkan/core/mixin.hpp"


namespace acma::vk {
    struct SIRIUS_API image_view : public mixin<VkImageView, PFN_vkDestroyImageView, logical_device> {
        // static result<image_view> create(sl::reference_ptr<const logical_device> device, image const& img, VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT) noexcept;
        //static result<image_view> create(sl::reference_ptr<const logical_device> device, VkImageViewCreateInfo create_info) noexcept;
	public:
		template<typename T>
		friend struct ::acma::impl::make;
    };
}



namespace acma::impl {
	template<>
    struct make<vk::image_view> {
		SIRIUS_API result<vk::image_view> operator()(
			sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
			sl::reference_ptr<const vk::logical_device> logi_device_ptr,
			VkImageViewCreateInfo create_infos,
			sl::in_place_adl_tag_type<vk::image_view> = sl::in_place_adl_tag<vk::image_view>
		) const noexcept;
	};
}