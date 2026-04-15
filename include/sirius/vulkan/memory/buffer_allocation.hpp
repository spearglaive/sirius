#pragma once
#include <streamline/functional/functor/generic_stateful.hpp>

#include "sirius/core/buffer_config.hpp"
#include "sirius/vulkan/memory/generic_allocation.hpp"

namespace acma::vk {
	using buffer_handle_t = VkBuffer;
	using buffer_creation_info_t = VkBufferCreateInfo;
}

namespace acma::vk {
	struct buffer_allocation : generic_allocation<buffer_handle_t, buffer_creation_info_t> {
		gpu_address_t device_address;
	};
}


namespace acma::vk::impl {
	using buffer_allocation_deleter = impl::allocation_deleter<buffer_allocation, &vmaDestroyBuffer>;
}

namespace acma::vk {
	using buffer_allocation_unique_ptr = sl::unique_ptr<buffer_allocation, impl::buffer_allocation_deleter>;
}