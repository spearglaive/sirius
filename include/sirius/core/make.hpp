#pragma once
#include <utility>
#include "sirius/core/error.hpp"

namespace acma::impl {
	template<typename T>
    struct make;
}


namespace acma {
	template<typename T>
	inline constexpr impl::make<T> make{};
}
