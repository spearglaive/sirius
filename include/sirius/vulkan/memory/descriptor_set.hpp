#pragma once
#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/api.def.h"
#include "sirius/vulkan/core/mixin.hpp"
#include "sirius/vulkan/core/command_pool.hpp"
#include "sirius/core/make.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/memory/descriptor_pool.hpp"


namespace acma::vk {
    struct SIRIUS_API descriptor_set : mixin<VkDescriptorSet> {//, PFN_vkFreeDescriptorSets, logical_device, descriptor_pool> {
	public:
		template<typename T>
		friend struct ::acma::impl::make;
	public:
		void update(std::span<const VkWriteDescriptorSet> writes) noexcept;
	private:
		sl::reference_ptr<const vk::function_table> vulkan_fns_ptr;
		sl::reference_ptr<const vk::logical_device> logi_device_ptr;
	};
}

namespace acma::impl {
	template<>
    struct make<vk::descriptor_set> {
		SIRIUS_API result<vk::descriptor_set> operator()(
			sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
			sl::reference_ptr<const vk::logical_device> logi_device_ptr,
			//sl::reference_ptr<const vk::descriptor_pool> descriptor_pool_ptr,
			VkDescriptorSetAllocateInfo create_info,
			sl::in_place_adl_tag_type<vk::descriptor_set> = sl::in_place_adl_tag<vk::descriptor_set>
		) const noexcept;
	};
}