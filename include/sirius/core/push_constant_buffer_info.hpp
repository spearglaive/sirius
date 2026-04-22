#pragma once
#include <streamline/numeric/int.hpp>

#include "sirius/core/buffer_key_t.hpp"


namespace acma {
	struct push_constant_buffer_info {
		buffer_key_t buffer_key;
		sl::uoffset_t offset = 0;
	};
}
