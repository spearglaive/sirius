#pragma once

#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/api.def.h"
#include "sirius/core/initialize.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/device/physical_device.hpp"
#include "sirius/vulkan/core/mixin.hpp"
#include "sirius/core/make.hpp"


namespace acma::vk {
    struct SIRIUS_API command_pool : mixin<VkCommandPool, PFN_vkDestroyCommandPool, logical_device> {
	public:
		template<typename T>
		friend struct ::acma::impl::make;
	};
}


namespace acma::impl {
	constexpr static sl::size_t command_pool_count = command_family::num_distinct_families + static_cast<sl::size_t>(impl::window_capability);

	template<>
    struct make<vk::command_pool> {
		SIRIUS_API result<vk::command_pool> operator()(
			sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
			sl::reference_ptr<const vk::physical_device> phys_device_ptr,
			sl::reference_ptr<const vk::logical_device> logi_device_ptr,
			command_family_t family,
			sl::in_place_adl_tag_type<vk::command_pool> = sl::in_place_adl_tag<vk::command_pool>
		) const noexcept;
	};
}