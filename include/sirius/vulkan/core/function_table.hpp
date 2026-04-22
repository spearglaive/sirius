#pragma once
#include <streamline/memory/reference_ptr.hpp>

#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/api.def.h"


namespace acma::vk {
	using function_table = VolkDeviceTable;
}


namespace acma {
	SIRIUS_API void make_function_table(
		vk::function_table& table,
		VkDevice device_handle
	) noexcept;
}
