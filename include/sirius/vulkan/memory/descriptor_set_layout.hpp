#pragma once
#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/api.def.h"
#include "sirius/vulkan/core/mixin.hpp"
#include "sirius/core/make.hpp"
#include "sirius/core/render_stage.hpp"
#include "sirius/vulkan/device/logical_device.hpp"


namespace acma::vk {
    struct SIRIUS_API descriptor_set_layout : mixin<VkDescriptorSetLayout, PFN_vkDestroyDescriptorSetLayout, logical_device> {};
}

namespace acma::impl {
	template<>
    struct make<vk::descriptor_set_layout> {
		SIRIUS_API result<vk::descriptor_set_layout> operator()(
			sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
			sl::reference_ptr<const vk::logical_device> logi_device_ptr,
			VkDescriptorSetLayoutCreateInfo create_info,
			sl::in_place_adl_tag_type<vk::descriptor_set_layout> = sl::in_place_adl_tag<vk::descriptor_set_layout>
		) const noexcept;
	};
}