#pragma once
#include <streamline/containers/array.hpp>
#include <streamline/containers/key_value_pair.hpp>
#include <streamline/metaprogramming/integer_sequence.hpp>
#include <streamline/numeric/int.hpp>

#include "sirius/core/asset_heap_key_t.hpp"
#include "sirius/core/buffer_key_t.hpp"
#include "sirius/core/push_constant_buffer_info.hpp"
#include "sirius/vulkan/memory/bind_point.hpp"


namespace acma {
	struct dispatch_info {
		buffer_key_t buffer_key;
		sl::uoffset_t offset = 0;
	};
}

namespace acma {
	struct dispatchable {
		constexpr static vk::bind_point_t pipeline_bind_point = vk::bind_point::compute;
	public:
		consteval static sl::uint32_t max_draw_count() noexcept { return static_cast<sl::uint32_t>(-1); }
	public:
		constexpr static sl::array<0, push_constant_buffer_info> push_constant_infos{};
	public:
		constexpr static sl::array<0, buffer_key_t> uniform_buffers{};
	public:
		constexpr static sl::array<0, dispatch_info> dispatch_infos{};
	};
}