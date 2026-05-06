#pragma once
#include <streamline/numeric/int.hpp>


namespace acma{
	using access_policy_t = sl::uint_fast8_t;
}

namespace acma {
	namespace access_policy {
	enum : access_policy_t {
		gpu_and_cpu,
		gpu_only,

		num_access_policies
	};
	}
}
