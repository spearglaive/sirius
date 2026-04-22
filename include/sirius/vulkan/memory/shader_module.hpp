#pragma once
#include <streamline/memory/reference_ptr.hpp>
#include <streamline/containers/array.hpp>
#include <array>

#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/api.def.h"
#include "sirius/vulkan/core/mixin.hpp"
#include "sirius/core/make.hpp"
#include "sirius/vulkan/device/logical_device.hpp"


namespace acma::vk {
    struct shader_module : mixin<VkShaderModule, PFN_vkDestroyShaderModule, logical_device> {
	public:
        VkPipelineShaderStageCreateInfo shader_stage_info;
    };
}


namespace acma::impl {
	template<>
    struct make<vk::shader_module> {
        template<sl::size_t N>
		SIRIUS_API result<vk::shader_module> operator()(
			sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
			sl::reference_ptr<const vk::logical_device> logi_device_ptr,
			std::array<unsigned char, N> data,
			VkShaderStageFlagBits stage,
			sl::in_place_adl_tag_type<vk::shader_module> = sl::in_place_adl_tag<vk::shader_module>
		) const noexcept;
	};
}

#include "sirius/vulkan/memory/shader_module.inl"