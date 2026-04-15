#pragma once
#include <cstdint>
#include "sirius/vulkan/core/vulkan.hpp"


namespace acma::vk {
	struct queue_family_info {
		bool supports_present : 1;
		std::uint32_t index : 31;
		std::uint32_t queue_count;
	};
}
