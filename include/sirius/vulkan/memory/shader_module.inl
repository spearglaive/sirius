#pragma once
#include "sirius/vulkan/memory/shader_module.hpp"

#include <streamline/memory/launder.hpp>


namespace acma::impl {
    template<std::size_t N>
	result<vk::shader_module>
		make<vk::shader_module>::
	operator()(
		sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
		sl::reference_ptr<const vk::logical_device> logi_device_ptr,
		std::array<unsigned char, N> data,
		VkShaderStageFlagBits stage,
		sl::in_place_adl_tag_type<vk::shader_module>
	) const noexcept {
        vk::shader_module ret{{{vulkan_fns_ptr->vkDestroyShaderModule, logi_device_ptr}}, VkPipelineShaderStageCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = stage,
            .module = VK_NULL_HANDLE,
            .pName = "main",
		}};

		//TODO verify alignment of passed shader data OR enforce constexpr
		const VkShaderModuleCreateInfo shader_module_create_info{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = data.size(),
            .pCode = sl::launder(reinterpret_cast<uint32_t const*>(data.data())),
        };
        __D2D_VULKAN_VERIFY(sl::invoke(vulkan_fns_ptr->vkCreateShaderModule, *logi_device_ptr, &shader_module_create_info, nullptr, &ret));
		ret.shader_stage_info.module = ret.smart_handle.get();

        return ret;
	}
}