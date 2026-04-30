#pragma once
#include <streamline/numeric/int.hpp>

#include "sirius/core/coupling_policy.hpp"
#include "sirius/core/memory_policy.hpp"


namespace acma::vk::impl {
	template<coupling_policy_t Coupling, memory_policy_t Memory>
	constexpr bool requires_clear = (
		Coupling == coupling_policy::decoupled &&
		memory_policy::is_cpu_writable(Memory)
	);
}



namespace acma::vk::impl {
	template<coupling_policy_t Coupling, memory_policy_t Memory>
	struct clear_frame {
		constexpr clear_frame& operator=(sl::index_t frame_number) noexcept {
			_val = frame_number;
			return *this;
		}

		constexpr bool matches(sl::index_t frame_number) noexcept {
			return _val >= frame_number;
		}
	public:
		sl::index_t _val;
	};
}


namespace acma::vk::impl {
	template<coupling_policy_t Coupling, memory_policy_t Memory>
	requires requires_clear<Coupling, Memory>
	struct clear_frame<Coupling, Memory> {
		constexpr clear_frame& operator=(sl::index_t) noexcept { return *this; }

		constexpr bool matches(sl::index_t) noexcept { return true; }
	};
}
