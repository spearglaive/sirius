#pragma once
#include <streamline/numeric/int.hpp>
#include <streamline/containers/array.hpp>


#include "sirius/core/command_family.hpp"

namespace acma::timeline {
	template<sl::size_t UserByteCount>
	struct state {
		void* user_pointer;
		[[no_unique_address]] sl::array<UserByteCount, sl::byte> user_bytes;
	public:
		sl::uint32_t image_index;
		sl::array<command_family::num_families, sl::uint32_t> queue_indices;
	};
}
