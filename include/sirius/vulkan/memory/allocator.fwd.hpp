#pragma once
#include "sirius/vulkan/core/vulkan.hpp"

#pragma GCC visibility push(default) //TEMP

#define VMA_VULKAN_VERSION 1003000 // Vulkan 1.3
#define VMA_STATIC_VULKAN_FUNCTIONS 1 //TODO: use volk and set this to 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include "vk_mem_alloc.h"

#pragma GCC visibility pop