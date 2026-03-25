#pragma once
#include <vector>
#include <streamline/numeric/int.hpp>

#include "sirius/graphics/core/texture_info.hpp"
#include "sirius/graphics/core/texture_view.hpp"


namespace acma {
	struct texture : texture_info {
		std::unique_ptr<sl::byte[]> bytes;
		sl::size_t bytes_count;

	public:
		constexpr operator texture_view() const& noexcept {
			return {*this, {bytes.get(), bytes_count}};
		};
	};
}