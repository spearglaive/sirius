#pragma once

#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/api.def.h"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/core/make.hpp"
#include "sirius/vulkan/core/unique_vk_ptr.hpp"
#include "sirius/vulkan/core/mixin.hpp"
#include "sirius/vulkan/display/image_view.hpp"


namespace acma::vk {
    struct SIRIUS_API image_view_handle :
        public image_view,
    	public mixin<VkImageView, PFN_vkDestroyImageView, logical_device>
    {

    public:
		template<typename T>
		friend struct ::acma::impl::make;

	public:
		constexpr operator VkDescriptorImageInfo() const noexcept {
			return {
				.imageView = *this,
				.imageLayout = this->layout()
			};
		}
    };
}



namespace acma::impl {
	template<>
    struct make<vk::image_view_handle> {
		SIRIUS_API result<vk::image_view_handle> operator()(
			sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
			sl::reference_ptr<const vk::logical_device> logi_device_ptr,
			vk::image_view associated_view,
			sl::in_place_adl_tag_type<vk::image_view_handle> = sl::in_place_adl_tag<vk::image_view_handle>
		) const noexcept;
	};
}
