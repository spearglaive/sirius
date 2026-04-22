#pragma once
#include <streamline/numeric/int.hpp>

#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/buffer_key_t.hpp"


namespace acma {
	struct index_buffer_info {
		buffer_key_t buffer_key;
		sl::uoffset_t offset = 0;
		sl::size_t index_size = sizeof(sl::uint16_t);
	};
}
