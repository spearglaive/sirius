#pragma once
#include <array>
#include <vector>
#include <streamline/functional/functor/generic_stateless.hpp>
#include <streamline/memory/unique_ptr.hpp>

#include <llfio.hpp>
#include <ktx.h>
#include <harfbuzz/hb.h>

#include "sirius/arith/point.hpp"
#include "sirius/core/error.hpp"
#include "sirius/graphics/core/texture.hpp"


namespace llfio = LLFIO_V2_NAMESPACE;

namespace acma {
	namespace decoder {
		namespace font_texture {
        	constexpr sl::size_t length_pixels = 32;
        	constexpr sl::size_t channels = 4;
        	constexpr sl::size_t size_bytes = length_pixels * length_pixels * channels;
        	constexpr double padding_em = 0.0625;
        	constexpr double distance_range = 0.125;
        	constexpr sl::size_t glyph_scale = length_pixels;
		}
	}


	namespace decoder {
		result<llfio::mapped_file_handle> open_file(llfio::path_view path) noexcept;
	}

	namespace decoder { 
		result<texture>
		decode_texture(llfio::mapped_file_handle const& handle, texture_usage usage) noexcept;
	};
}
