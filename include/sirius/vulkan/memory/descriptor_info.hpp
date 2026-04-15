#pragma once

#include "sirius/vulkan/core/vulkan.hpp"


namespace acma::vk {
	union descriptor_info {
		VkDescriptorImageInfo image;
		VkDescriptorBufferInfo buffer;
	};
}