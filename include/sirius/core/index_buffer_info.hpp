#pragma once
#include <streamline/numeric/int.hpp>

#include "sirius/core/buffer_info.hpp"
#include "sirius/core/buffer_key_t.hpp"


namespace acma {
	struct index_buffer_info : basic_buffer_info {
		sl::size_t index_size = sizeof(sl::uint16_t);
	};
}
