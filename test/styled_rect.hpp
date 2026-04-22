#pragma once
#include <array>

#include <streamline/metaprogramming/integer_sequence.hpp>

#include "sirius/core/drawable.hpp"
#include "sirius/shaders/rect.hpp"

#include "./buffer_config_table.hpp"
#include "./asset_heap_config_table.hpp"

namespace acma::test {
	struct styled_rect : public acma::drawable {
        constexpr static auto vert_shader_data = std::to_array(acma::shaders::rect::vert);
        constexpr static auto frag_shader_data = std::to_array(acma::shaders::rect::frag);

	public:
		constexpr static index_buffer_info index_info{::buffer_id::rectangle_indices};
		constexpr static sl::array<1, push_constant_buffer_info> push_constant_infos{{
			{::buffer_id::draw_constants},
		}};
	public:
		constexpr static sl::array<1, draw_info> draw_infos{{
			{::buffer_id::draw_commands, ::buffer_id::counts}
		}};
	};
}