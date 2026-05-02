#pragma once
#include <streamline/numeric/int.hpp>

#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/buffer_key_t.hpp"


namespace acma {
	struct basic_buffer_info {
		buffer_key_t buffer_key;
		sl::uoffset_t offset = 0;
	};

	struct buffer_info {
		buffer_key_t buffer_key;
		sl::uoffset_t offset = 0;
		sl::size_t size = VK_WHOLE_SIZE;
	};
}
