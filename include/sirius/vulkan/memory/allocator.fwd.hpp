#pragma once
#include "sirius/vulkan/core/vulkan.hpp"
#include "sirius/core/api.def.h"


//#define VMA_CALL_PRE SIRIUS_API
#define VMA_VULKAN_VERSION 1003000 // Vulkan 1.3
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include "vk_mem_alloc.h"



//Forward declares (for visibility) [possibly just a temporary solution]
#undef VMA_CALL_PRE
#define VMA_CALL_PRE SIRIUS_API

extern "C" {
	VMA_CALL_PRE VkResult VMA_CALL_POST vmaImportVulkanFunctionsFromVolk(
		const VmaAllocatorCreateInfo* VMA_NOT_NULL pAllocatorCreateInfo,
		VmaVulkanFunctions* VMA_NOT_NULL pDstVulkanFunctions
	);
	
	VMA_CALL_PRE VkResult VMA_CALL_POST vmaCreateAllocator(
		const VmaAllocatorCreateInfo* VMA_NOT_NULL pCreateInfo,
		VmaAllocator VMA_NULLABLE* VMA_NOT_NULL pAllocator
	);

	VMA_CALL_PRE void VMA_CALL_POST vmaDestroyAllocator(
		VmaAllocator VMA_NULLABLE allocator
	);

	VMA_CALL_PRE VkResult VMA_CALL_POST vmaAllocateMemory(
		VmaAllocator VMA_NOT_NULL allocator,
		const VkMemoryRequirements* VMA_NOT_NULL pVkMemoryRequirements,
		const VmaAllocationCreateInfo* VMA_NOT_NULL pCreateInfo,
		VmaAllocation VMA_NULLABLE* VMA_NOT_NULL pAllocation,
		VmaAllocationInfo* VMA_NULLABLE pAllocationInfo
	);

	VMA_CALL_PRE void VMA_CALL_POST vmaFreeMemory(
		VmaAllocator VMA_NOT_NULL allocator,
		VmaAllocation VMA_NULLABLE allocation
	);

	VMA_CALL_PRE VkResult VMA_CALL_POST vmaCreateBuffer(
		VmaAllocator VMA_NOT_NULL allocator,
		const VkBufferCreateInfo* VMA_NOT_NULL pBufferCreateInfo,
		const VmaAllocationCreateInfo* VMA_NOT_NULL pAllocationCreateInfo,
		VkBuffer VMA_NULLABLE_NON_DISPATCHABLE* VMA_NOT_NULL pBuffer,
		VmaAllocation VMA_NULLABLE* VMA_NOT_NULL pAllocation,
		VmaAllocationInfo* VMA_NULLABLE pAllocationInfo
	);

	VMA_CALL_PRE VkResult VMA_CALL_POST vmaCreateAliasingBuffer2(
		VmaAllocator VMA_NOT_NULL allocator,
		VmaAllocation VMA_NOT_NULL allocation,
		VkDeviceSize allocationLocalOffset,
		const VkBufferCreateInfo* VMA_NOT_NULL pBufferCreateInfo,
		VkBuffer VMA_NULLABLE_NON_DISPATCHABLE* VMA_NOT_NULL pBuffer
	);

	VMA_CALL_PRE void VMA_CALL_POST vmaDestroyBuffer(
		VmaAllocator VMA_NOT_NULL allocator,
		VkBuffer VMA_NULLABLE_NON_DISPATCHABLE buffer,
		VmaAllocation VMA_NULLABLE allocation
	);

	VMA_CALL_PRE VkResult VMA_CALL_POST vmaCreateImage(
		VmaAllocator VMA_NOT_NULL allocator,
		const VkImageCreateInfo* VMA_NOT_NULL pImageCreateInfo,
		const VmaAllocationCreateInfo* VMA_NOT_NULL pAllocationCreateInfo,
		VkImage VMA_NULLABLE_NON_DISPATCHABLE* VMA_NOT_NULL pImage,
		VmaAllocation VMA_NULLABLE* VMA_NOT_NULL pAllocation,
		VmaAllocationInfo* VMA_NULLABLE pAllocationInfo
	);

	VMA_CALL_PRE void VMA_CALL_POST vmaDestroyImage(
		VmaAllocator VMA_NOT_NULL allocator,
		VkImage VMA_NULLABLE_NON_DISPATCHABLE image,
		VmaAllocation VMA_NULLABLE allocation
	);
}

#undef VMA_CALL_PRE
#define VMA_CALL_PRE
