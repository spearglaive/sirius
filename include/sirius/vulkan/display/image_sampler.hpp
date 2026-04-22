#pragma once

#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/api.def.h"
#include "sirius/arith/point.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/device/physical_device.hpp"


namespace acma::vk {
    struct SIRIUS_API image_sampler : public mixin<VkSampler, PFN_vkDestroySampler, logical_device> {
    public:
        pt3<VkSamplerAddressMode> address_modes;
    };
}


namespace acma::impl {
	template<>
    struct make<vk::image_sampler> {
		SIRIUS_API result<vk::image_sampler> operator()(
			sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
			sl::reference_ptr<const vk::logical_device> logi_device_ptr,
			float max_anisotropy,
			pt3<VkSamplerAddressMode> address_modes = {
				VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
				VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
				VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
			},
			sl::in_place_adl_tag_type<vk::image_sampler> = sl::in_place_adl_tag<vk::image_sampler>
		) const noexcept;


		SIRIUS_API result<vk::image_sampler> operator()(
			sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
			sl::reference_ptr<const vk::logical_device> logi_device_ptr,
			VkSamplerCreateInfo create_info,
			sl::in_place_adl_tag_type<vk::image_sampler> = sl::in_place_adl_tag<vk::image_sampler>
		) const noexcept;
	};
}