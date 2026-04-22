#pragma once
#include <memory>
#include <streamline/containers/tuple.hpp>
#include <streamline/memory/reference_ptr.hpp>

#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/make.hpp"
#include "sirius/vulkan/core/unique_vk_ptr.hpp"
#include "sirius/vulkan/core/mixin.hpp"
#include "sirius/core/api.def.h"
#include "sirius/core/command_family.hpp"
#include "sirius/vulkan/device/physical_device.hpp"

namespace acma::vk {
    struct SIRIUS_API logical_device : mixin<VkDevice, PFN_vkDestroyDevice> {
		// void initialize(
		// 	sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
		// 	sl::reference_ptr<const vk::physical_device> associated_phys_device_ptr
		// ) noexcept;
	public:
		template<typename T>
		friend struct ::acma::impl::make;

    public:
        std::array<std::vector<VkQueue>, command_family::num_families> queues;
    };
}


namespace acma::impl {
	template<>
    struct make<vk::logical_device> {
		SIRIUS_API result<vk::logical_device> operator()(
			sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
			sl::reference_ptr<const vk::physical_device> associated_phys_device_ptr,
			bool windowing,
			sl::in_place_adl_tag_type<vk::logical_device> = sl::in_place_adl_tag<vk::logical_device>
		) const noexcept;
	};
}