#include "sirius/vulkan/core/function_table.hpp"

namespace acma {
	void make_function_table(
		vk::function_table& table,
		VkDevice device_handle
	) noexcept {
		return volkLoadDeviceTable(&table, device_handle);
	}
}