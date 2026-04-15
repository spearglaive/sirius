#pragma once
#include <streamline/functional/functor/generic_stateful.hpp>

#include "sirius/vulkan/memory/generic_allocation.hpp"

namespace acma::vk {
	using image_handle_t = VkImage;
	using image_creation_info_t = VkImageCreateInfo;
}

namespace acma::vk {
	using image_allocation = generic_allocation<image_handle_t, image_creation_info_t>;
}

namespace acma::vk::impl {
	using image_allocation_deleter =  impl::allocation_deleter<image_allocation, &vmaDestroyImage>;
}

namespace acma::vk {
	using image_allocation_unique_ptr = sl::unique_ptr<image_allocation, impl::image_allocation_deleter>;
}