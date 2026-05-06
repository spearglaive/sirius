#pragma once
#include <streamline/containers/array.hpp>

#include "sirius/core/access_policy.hpp"
#include "sirius/core/frames_in_flight.def.hpp"


namespace acma::vk::impl {
	constexpr sl::array<access_policy::num_access_policies, sl::size_t> allocation_counts{{D2D_FRAMES_IN_FLIGHT, 1}};
}
