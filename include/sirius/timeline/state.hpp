#pragma once
#include <streamline/numeric/int.hpp>
#include <streamline/containers/array.hpp>


#include "sirius/core/command_family.hpp"

namespace acma::timeline {
	struct state {
		sl::uint32_t image_index;
		sl::array<command_family::num_families, sl::uint32_t> queue_indices;
	};
}