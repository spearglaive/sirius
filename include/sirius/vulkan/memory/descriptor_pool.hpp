#pragma once
#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/api.def.h"
#include "sirius/vulkan/core/mixin.hpp"
#include "sirius/core/make.hpp"
#include "sirius/core/render_stage.hpp"
#include "sirius/vulkan/device/logical_device.hpp"


namespace acma::vk {
    struct SIRIUS_API descriptor_pool : mixin<VkDescriptorPool, PFN_vkDestroyDescriptorPool, logical_device> {};
}

namespace acma::impl {
	template<>
    struct make<vk::descriptor_pool> {
		SIRIUS_API result<vk::descriptor_pool> operator()(
			sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
			sl::reference_ptr<const vk::logical_device> logi_device_ptr,
			VkDescriptorPoolCreateInfo create_info,
			sl::in_place_adl_tag_type<vk::descriptor_pool> = sl::in_place_adl_tag<vk::descriptor_pool>
		) const noexcept;
	};
}