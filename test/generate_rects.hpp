#pragma once
#include <array>

#include <streamline/metaprogramming/integer_sequence.hpp>

#include "sirius/core/dispatchable.hpp"
#include "sirius/shaders/rect.hpp"
#include "sirius/shaders/generate_rects.hpp"

#include "./buffer_config_table.hpp"
#include "./asset_heap_config_table.hpp"

namespace acma::test {
	struct generate_rects : public acma::dispatchable {
        constexpr static auto comp_shader_data = std::to_array(acma::shaders::generate_rects::comp);
	public:
		constexpr static sl::array<1, buffer_info> push_constant_infos{{
			{::buffer_id::compute_constants}
		}};
	public:
		constexpr static sl::array<1, buffer_info> uniform_infos{{
			{::buffer_id::offset}
		}};
	public:
		constexpr static sl::array<1, dispatch_info> dispatch_infos{{
			{::buffer_id::dispatch_commands}
		}};
	};
}
