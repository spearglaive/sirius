#pragma once
// #include <volk.h> //TODO

#include <vulkan/vulkan.h>

namespace acma::vk {
	struct function_table {
		PFN_vkCreateDevice vkCreateDevice = &::vkCreateDevice;
		PFN_vkDestroyDevice vkDestroyDevice = &::vkDestroyDevice;
		PFN_vkGetDeviceQueue vkGetDeviceQueue = &::vkGetDeviceQueue;
		PFN_vkDeviceWaitIdle vkDeviceWaitIdle  = &::vkDeviceWaitIdle;

		PFN_vkCreateImageView vkCreateImageView = &::vkCreateImageView;
		PFN_vkDestroyImageView vkDestroyImageView = &::vkDestroyImageView;

		PFN_vkCreateCommandPool vkCreateCommandPool = &::vkCreateCommandPool;
		PFN_vkDestroyCommandPool vkDestroyCommandPool = &::vkDestroyCommandPool;

		PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers = &::vkAllocateCommandBuffers;
		PFN_vkFreeCommandBuffers vkFreeCommandBuffers = &::vkFreeCommandBuffers;

		PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR = &::vkAcquireNextImageKHR;
		PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR = &::vkCreateSwapchainKHR;
		PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR = &::vkDestroySwapchainKHR;
		PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR = &::vkGetSwapchainImagesKHR;
		PFN_vkQueuePresentKHR vkQueuePresentKHR = &::vkQueuePresentKHR;

		PFN_vkGetBufferDeviceAddress vkGetBufferDeviceAddress = &::vkGetBufferDeviceAddress;

		PFN_vkCreateSemaphore vkCreateSemaphore = &::vkCreateSemaphore;
		PFN_vkDestroySemaphore vkDestroySemaphore = &::vkDestroySemaphore;
		PFN_vkWaitSemaphores vkWaitSemaphores = &::vkWaitSemaphores;

		PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout = &::vkCreateDescriptorSetLayout;
		PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout = &::vkDestroyDescriptorSetLayout;
		PFN_vkCreateDescriptorPool vkCreateDescriptorPool = &::vkCreateDescriptorPool;
		PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool = &::vkDestroyDescriptorPool;
		PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets = &::vkAllocateDescriptorSets;
		PFN_vkFreeDescriptorSets vkFreeDescriptorSets = &::vkFreeDescriptorSets;
		PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets = &::vkUpdateDescriptorSets;

		PFN_vkCreatePipelineLayout vkCreatePipelineLayout = &::vkCreatePipelineLayout;
		PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout = &::vkDestroyPipelineLayout;

		PFN_vkCreateShaderModule vkCreateShaderModule = &::vkCreateShaderModule;
		PFN_vkDestroyShaderModule vkDestroyShaderModule = &::vkDestroyShaderModule;

		PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines = &::vkCreateGraphicsPipelines;
		PFN_vkCreateComputePipelines vkCreateComputePipelines = &::vkCreateComputePipelines;
		PFN_vkDestroyPipeline vkDestroyPipeline = &::vkDestroyPipeline;

		PFN_vkCreateSampler vkCreateSampler = &::vkCreateSampler;
		PFN_vkDestroySampler vkDestroySampler = &::vkDestroySampler;

		PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR = nullptr;

		PFN_vkCmdBeginQuery vkCmdBeginQuery = &::vkCmdBeginQuery;
		PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass = &::vkCmdBeginRenderPass;
		PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets = &::vkCmdBindDescriptorSets;
		PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer = &::vkCmdBindIndexBuffer;
		PFN_vkCmdBindPipeline vkCmdBindPipeline = &::vkCmdBindPipeline;
		PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers = &::vkCmdBindVertexBuffers;
		PFN_vkCmdBlitImage vkCmdBlitImage = &::vkCmdBlitImage;
		PFN_vkCmdClearAttachments vkCmdClearAttachments = &::vkCmdClearAttachments;
		PFN_vkCmdClearColorImage vkCmdClearColorImage = &::vkCmdClearColorImage;
		PFN_vkCmdClearDepthStencilImage vkCmdClearDepthStencilImage = &::vkCmdClearDepthStencilImage;
		PFN_vkCmdCopyBuffer vkCmdCopyBuffer = &::vkCmdCopyBuffer;
		PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage = &::vkCmdCopyBufferToImage;
		PFN_vkCmdCopyImage vkCmdCopyImage = &::vkCmdCopyImage;
		PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer = &::vkCmdCopyImageToBuffer;
		PFN_vkCmdCopyQueryPoolResults vkCmdCopyQueryPoolResults = &::vkCmdCopyQueryPoolResults;
		PFN_vkCmdDispatch vkCmdDispatch = &::vkCmdDispatch;
		PFN_vkCmdDispatchIndirect vkCmdDispatchIndirect = &::vkCmdDispatchIndirect;
		PFN_vkCmdDraw vkCmdDraw = &::vkCmdDraw;
		PFN_vkCmdDrawIndexed vkCmdDrawIndexed = &::vkCmdDrawIndexed;
		PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect = &::vkCmdDrawIndexedIndirect;
		PFN_vkCmdDrawIndirect vkCmdDrawIndirect = &::vkCmdDrawIndirect;
		PFN_vkCmdEndQuery vkCmdEndQuery = &::vkCmdEndQuery;
		PFN_vkCmdEndRenderPass vkCmdEndRenderPass = &::vkCmdEndRenderPass;
		PFN_vkCmdExecuteCommands vkCmdExecuteCommands = &::vkCmdExecuteCommands;
		PFN_vkCmdFillBuffer vkCmdFillBuffer = &::vkCmdFillBuffer;
		PFN_vkCmdNextSubpass vkCmdNextSubpass = &::vkCmdNextSubpass;
		PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier = &::vkCmdPipelineBarrier;
		PFN_vkCmdPushConstants vkCmdPushConstants = &::vkCmdPushConstants;
		PFN_vkCmdResetEvent vkCmdResetEvent = &::vkCmdResetEvent;
		PFN_vkCmdResetQueryPool vkCmdResetQueryPool = &::vkCmdResetQueryPool;
		PFN_vkCmdResolveImage vkCmdResolveImage = &::vkCmdResolveImage;
		PFN_vkCmdSetBlendConstants vkCmdSetBlendConstants = &::vkCmdSetBlendConstants;
		PFN_vkCmdSetDepthBias vkCmdSetDepthBias = &::vkCmdSetDepthBias;
		PFN_vkCmdSetDepthBounds vkCmdSetDepthBounds = &::vkCmdSetDepthBounds;
		PFN_vkCmdSetEvent vkCmdSetEvent = &::vkCmdSetEvent;
		PFN_vkCmdSetLineWidth vkCmdSetLineWidth = &::vkCmdSetLineWidth;
		PFN_vkCmdSetScissor vkCmdSetScissor = &::vkCmdSetScissor;
		PFN_vkCmdSetStencilCompareMask vkCmdSetStencilCompareMask = &::vkCmdSetStencilCompareMask;
		PFN_vkCmdSetStencilReference vkCmdSetStencilReference = &::vkCmdSetStencilReference;
		PFN_vkCmdSetStencilWriteMask vkCmdSetStencilWriteMask = &::vkCmdSetStencilWriteMask;
		PFN_vkCmdSetViewport vkCmdSetViewport = &::vkCmdSetViewport;
		PFN_vkCmdUpdateBuffer vkCmdUpdateBuffer = &::vkCmdUpdateBuffer;
		PFN_vkCmdWaitEvents vkCmdWaitEvents = &::vkCmdWaitEvents;
		PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp = &::vkCmdWriteTimestamp;
		PFN_vkCmdBeginRenderPass2 vkCmdBeginRenderPass2 = &::vkCmdBeginRenderPass2;
		PFN_vkCmdDrawIndexedIndirectCount vkCmdDrawIndexedIndirectCount = &::vkCmdDrawIndexedIndirectCount;
		PFN_vkCmdDrawIndirectCount vkCmdDrawIndirectCount = &::vkCmdDrawIndirectCount;
		PFN_vkCmdEndRenderPass2 vkCmdEndRenderPass2 = &::vkCmdEndRenderPass2;
		PFN_vkCmdNextSubpass2 vkCmdNextSubpass2 = &::vkCmdNextSubpass2;
		PFN_vkCmdBeginRendering vkCmdBeginRendering = &::vkCmdBeginRendering;
		PFN_vkCmdBindVertexBuffers2 vkCmdBindVertexBuffers2 = &::vkCmdBindVertexBuffers2;
		PFN_vkCmdBlitImage2 vkCmdBlitImage2 = &::vkCmdBlitImage2;
		PFN_vkCmdCopyBuffer2 vkCmdCopyBuffer2 = &::vkCmdCopyBuffer2;
		PFN_vkCmdCopyBufferToImage2 vkCmdCopyBufferToImage2 = &::vkCmdCopyBufferToImage2;
		PFN_vkCmdCopyImage2 vkCmdCopyImage2 = &::vkCmdCopyImage2;
		PFN_vkCmdCopyImageToBuffer2 vkCmdCopyImageToBuffer2 = &::vkCmdCopyImageToBuffer2;
		PFN_vkCmdEndRendering vkCmdEndRendering = &::vkCmdEndRendering;
		PFN_vkCmdPipelineBarrier2 vkCmdPipelineBarrier2 = &::vkCmdPipelineBarrier2;
		PFN_vkCmdResetEvent2 vkCmdResetEvent2 = &::vkCmdResetEvent2;
		PFN_vkCmdResolveImage2 vkCmdResolveImage2 = &::vkCmdResolveImage2;
		PFN_vkCmdSetCullMode vkCmdSetCullMode = &::vkCmdSetCullMode;
		PFN_vkCmdSetDepthBiasEnable vkCmdSetDepthBiasEnable = &::vkCmdSetDepthBiasEnable;
		PFN_vkCmdSetDepthBoundsTestEnable vkCmdSetDepthBoundsTestEnable = &::vkCmdSetDepthBoundsTestEnable;
		PFN_vkCmdSetDepthCompareOp vkCmdSetDepthCompareOp = &::vkCmdSetDepthCompareOp;
		PFN_vkCmdSetDepthTestEnable vkCmdSetDepthTestEnable = &::vkCmdSetDepthTestEnable;
		PFN_vkCmdSetDepthWriteEnable vkCmdSetDepthWriteEnable = &::vkCmdSetDepthWriteEnable;
		PFN_vkCmdSetEvent2 vkCmdSetEvent2 = &::vkCmdSetEvent2;
		PFN_vkCmdSetFrontFace vkCmdSetFrontFace = &::vkCmdSetFrontFace;
		PFN_vkCmdSetPrimitiveRestartEnable vkCmdSetPrimitiveRestartEnable = &::vkCmdSetPrimitiveRestartEnable;
		PFN_vkCmdSetPrimitiveTopology vkCmdSetPrimitiveTopology = &::vkCmdSetPrimitiveTopology;
		PFN_vkCmdSetRasterizerDiscardEnable vkCmdSetRasterizerDiscardEnable = &::vkCmdSetRasterizerDiscardEnable;
		PFN_vkCmdSetScissorWithCount vkCmdSetScissorWithCount = &::vkCmdSetScissorWithCount;
		PFN_vkCmdSetStencilOp vkCmdSetStencilOp = &::vkCmdSetStencilOp;
		PFN_vkCmdSetStencilTestEnable vkCmdSetStencilTestEnable = &::vkCmdSetStencilTestEnable;
		PFN_vkCmdSetViewportWithCount vkCmdSetViewportWithCount = &::vkCmdSetViewportWithCount;
		PFN_vkCmdWaitEvents2 vkCmdWaitEvents2 = &::vkCmdWaitEvents2;
		PFN_vkCmdWriteTimestamp2 vkCmdWriteTimestamp2 = &::vkCmdWriteTimestamp2;
	};
}