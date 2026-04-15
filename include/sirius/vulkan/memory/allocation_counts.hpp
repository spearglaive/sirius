#pragma once
#include <streamline/containers/array.hpp>

#include "sirius/core/coupling_policy.hpp"
#include "sirius/core/frames_in_flight.def.hpp"


namespace acma::vk::impl {
	constexpr sl::array<coupling_policy::num_coupling_policies, sl::size_t> allocation_counts{{D2D_FRAMES_IN_FLIGHT, 1}};
}
