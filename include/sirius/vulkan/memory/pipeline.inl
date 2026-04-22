#pragma once
#include "sirius/vulkan/memory/pipeline.hpp"

#include "sirius/core/make.hpp"
#include "sirius/vulkan/memory/pipeline_layout.hpp"
#include "sirius/vulkan/memory/shader_module.hpp"
#include "sirius/vulkan/core/vulkan.hpp"


namespace acma::impl {
	template<typename T, auto BufferConfigs, auto AssetHeapConfigs>
	template<typename RenderProcessT>
	result<vk::pipeline<vk::bind_point::graphics, T, BufferConfigs, AssetHeapConfigs>>
		make<vk::pipeline<vk::bind_point::graphics, T, BufferConfigs, AssetHeapConfigs>>::
	operator()(
		sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
		sl::reference_ptr<const vk::logical_device> logi_device_ptr,
		RenderProcessT&& proc,
		std::span<const VkFormat> color_attachment_formats,
		VkFormat depth_attachment_format,
		sl::in_place_adl_tag_type<vk::pipeline<vk::bind_point::graphics, T, BufferConfigs, AssetHeapConfigs>>
	) const noexcept {
		vk::pipeline<vk::bind_point::graphics, T, BufferConfigs, AssetHeapConfigs> ret{};
		ret.smart_handle = {vulkan_fns_ptr->vkDestroyPipeline, logi_device_ptr};

		RESULT_TRY_MOVE(ret._layout, (acma::make<vk::pipeline_layout<shader_stage::all_graphics, T, BufferConfigs, AssetHeapConfigs>>(
			vulkan_fns_ptr,
			logi_device_ptr,
			sl::forward<RenderProcessT>(proc)
		)))

		//TODO: make these the default values but allow developer to change any of these

		//Specify vertex input state (always empty because of BDA)
		constexpr static VkPipelineVertexInputStateCreateInfo vertex_input_info {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount = 0,
			.pVertexBindingDescriptions = nullptr,
			.vertexAttributeDescriptionCount = 0,
			.pVertexAttributeDescriptions = nullptr,
		};

		//Specify input assembly
		const VkPipelineInputAssemblyStateCreateInfo input_assembly_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE,
		};

		//Specify viewport and scissor state
		const VkPipelineViewportStateCreateInfo viewport_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.scissorCount = 1,
		};

		//Specify dynamic states
		constexpr static std::array<VkDynamicState, 2> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};//, VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY};
		const VkPipelineDynamicStateCreateInfo dynamic_state_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = dynamic_states.size(),
			.pDynamicStates = dynamic_states.data(),
		};

		//Create basic rasterizer
		const VkPipelineRasterizationStateCreateInfo rasterizer_info {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_BACK_BIT,//T::cull_mode,
			.frontFace = VK_FRONT_FACE_CLOCKWISE,//T::front_face,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.0f,
			.depthBiasClamp = 0.0f,
			.depthBiasSlopeFactor = 0.0f,
			.lineWidth = 1.0f,
		};

		//Specify multi-sampling (TEMP: no multi-sampling)
		const VkPipelineMultisampleStateCreateInfo multisampling_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE,
		};

		//Specify basic color blend
		const VkPipelineColorBlendAttachmentState color_blend_attach_info{
			.blendEnable = VK_TRUE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
			.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			.colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			.alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		};
	   	const VkPipelineColorBlendStateCreateInfo color_blend_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 1,
			.pAttachments = &color_blend_attach_info,
			.blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
		};

		//Specify depth/stencil
		const VkPipelineDepthStencilStateCreateInfo depth_and_stencil_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.depthTestEnable = VK_TRUE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
			.front = VkStencilOpState{},
			.back = VkStencilOpState(),
			.minDepthBounds = 0.0f,
			.maxDepthBounds = 1.0f
		};

		//Specify rendering info
		const VkPipelineRenderingCreateInfo rendering_info {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.colorAttachmentCount = static_cast<std::uint32_t>(color_attachment_formats.size()),
			.pColorAttachmentFormats = color_attachment_formats.data(),
			.depthAttachmentFormat = depth_attachment_format
		};


		RESULT_TRY_MOVE_UNSCOPED(const vk::shader_module vert_shader, acma::make<vk::shader_module>(vulkan_fns_ptr, logi_device_ptr, T::vert_shader_data, VK_SHADER_STAGE_VERTEX_BIT), vs);
		RESULT_TRY_MOVE_UNSCOPED(const vk::shader_module frag_shader, acma::make<vk::shader_module>(vulkan_fns_ptr, logi_device_ptr, T::frag_shader_data, VK_SHADER_STAGE_FRAGMENT_BIT), fs);
		const sl::array<2, VkPipelineShaderStageCreateInfo> shader_stages = {vert_shader.shader_stage_info, frag_shader.shader_stage_info};

		//VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags{
		//	.sType = VK_STRUCTURE_TYPE_PIPELINE_CREATE_FLAGS_2_CREATE_INFO_KHR,
		//	.pNext = &rendering_info,
		//	.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT,
		//};
		const VkGraphicsPipelineCreateInfo pipeline_create_info{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = &rendering_info, //&pipeline_create_flags,
			.flags = VkPipelineCreateFlags{},
			.stageCount = shader_stages.size(),
			.pStages = shader_stages.data(),
			.pVertexInputState = &vertex_input_info,
			.pInputAssemblyState = &input_assembly_info,
			.pTessellationState = nullptr,
			.pViewportState = &viewport_info,
			.pRasterizationState = &rasterizer_info,
			.pMultisampleState = &multisampling_info,
			.pDepthStencilState = &depth_and_stencil_info,
			.pColorBlendState = &color_blend_info,
			.pDynamicState = &dynamic_state_info,
			.layout = ret._layout,
			.renderPass = VK_NULL_HANDLE,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex = -1,
		};
		__D2D_VULKAN_VERIFY(sl::invoke(vulkan_fns_ptr->vkCreateGraphicsPipelines, *logi_device_ptr, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &ret));
		return ret;
	}
}


namespace acma::impl {
	template<typename T, auto BufferConfigs, auto AssetHeapConfigs>
	template<typename RenderProcessT>
	result<vk::pipeline<vk::bind_point::compute, T, BufferConfigs, AssetHeapConfigs>>
		make<vk::pipeline<vk::bind_point::compute, T, BufferConfigs, AssetHeapConfigs>>::
	operator()(
		sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
		sl::reference_ptr<const vk::logical_device> logi_device_ptr,
		RenderProcessT&& proc,
		sl::in_place_adl_tag_type<vk::pipeline<vk::bind_point::compute, T, BufferConfigs, AssetHeapConfigs>>
	) const noexcept {
		vk::pipeline<vk::bind_point::compute, T, BufferConfigs, AssetHeapConfigs> ret{};
		ret.smart_handle = {vulkan_fns_ptr->vkDestroyPipeline, logi_device_ptr};

		RESULT_TRY_MOVE(ret._layout, (acma::make<vk::pipeline_layout<shader_stage::compute, T, BufferConfigs, AssetHeapConfigs>>(
			vulkan_fns_ptr,
			logi_device_ptr,
			sl::forward<RenderProcessT>(proc)
		)));

		RESULT_TRY_MOVE_UNSCOPED(const vk::shader_module comp_shader, acma::make<vk::shader_module>(
			vulkan_fns_ptr,
			logi_device_ptr,
			T::comp_shader_data,
			VK_SHADER_STAGE_COMPUTE_BIT
		), cs);

		//VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags{
		//	.sType = VK_STRUCTURE_TYPE_PIPELINE_CREATE_FLAGS_2_CREATE_INFO_KHR,
		//	.pNext = nullptr,
		//	.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT,
		//};
		const VkComputePipelineCreateInfo pipeline_create_info{
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.pNext = nullptr, //&pipeline_create_flags,
			.flags = VkPipelineCreateFlags{},
			.stage = comp_shader.shader_stage_info,
			.layout = ret._layout,
			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex = -1,
		};
		__D2D_VULKAN_VERIFY(sl::invoke(vulkan_fns_ptr->vkCreateComputePipelines, *logi_device_ptr, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &ret));
		return ret;
	}
}
