#include "sirius/core/decoder.hpp"

#include <harfbuzz/hb.h>
#include <ktx.h>
#include <ktxvulkan.h>
#include <msdfgen.h>
#include <msdfgen/core/DistanceMapping.h>
#include <msdfgen/core/ShapeDistanceFinder.h>
#include <msdfgen/core/edge-selectors.h>


namespace acma::decoder {
	result<llfio::mapped_file_handle> open_file(llfio::path_view path) noexcept {
        auto mf = llfio::mapped_file({}, path);
        if(mf.has_error())
            return static_cast<errc>(mf.assume_error().value());
        llfio::mapped_file_handle mh = std::move(mf).assume_value();

        auto extent = mh.maximum_extent();
        if(extent.has_error())
            return static_cast<errc>(extent.assume_error().value());

		return sl::move(mh);
	}
}

namespace acma::decoder { 
	result<texture>
	decode_texture(llfio::mapped_file_handle const& handle, texture_usage usage) noexcept {
		using texture_unique_ptr = sl::unique_ptr<ktxTexture2, sl::functor::generic_stateless<ktxTexture2_Destroy>>;
		texture_unique_ptr ktx_ptr = sl::make_default<texture_unique_ptr>(sl::in_place_tag);
        __D2D_KTX_VERIFY(ktxTexture2_CreateFromMemory(reinterpret_cast<ktx_uint8_t const*>(handle.address()), handle.maximum_extent().assume_value(), KTX_TEXTURE_CREATE_NO_FLAGS, &ktx_ptr.get()));


        //TODO: compressed format transcoding
        const bool transcoded = ktxTexture2_NeedsTranscoding(ktx_ptr.get());
        if (transcoded) 
			__D2D_KTX_VERIFY(ktxTexture2_TranscodeBasis(ktx_ptr.get(), KTX_TTF_RGBA32, 0));


		const sl::size_t data_size = ktxTexture_GetDataSizeUncompressed(ktxTexture(ktx_ptr.get()));
		std::unique_ptr<sl::byte[]> bytes = std::make_unique_for_overwrite<sl::byte[]>(data_size);

		//ktxTexture2_TranscodeBasis already loads the image, so calling ktxTexture2_LoadImageData afterwards will fail
		if(transcoded)
			std::memcpy(bytes.get(), ktxTexture_GetData(ktxTexture(ktx_ptr.get())), data_size);
		else 
			__D2D_KTX_VERIFY(ktxTexture2_LoadImageData(ktx_ptr.get(), reinterpret_cast<ktx_uint8_t*>(bytes.get()), data_size));
		

		sl::array<max_mip_levels, sl::uoffset_t> mip_offsets;
		for(sl::index_t i = 0; i < std::min(static_cast<ktx_uint32_t>(max_mip_levels), ktx_ptr->numLevels); ++i) {
			__D2D_KTX_VERIFY(ktxTexture2_GetImageOffset(ktx_ptr.get(), i, 0, 0, &mip_offsets[i]));
		}

		return texture{
			texture_info{
				.dimensions = ktx_ptr->numDimensions,
				.format_id = ktxTexture2_GetVkFormat(ktx_ptr.get()),
				.extent = {ktx_ptr->baseWidth, ktx_ptr->baseHeight, ktx_ptr->baseDepth},
				.mip_level_count = ktx_ptr->numLevels,
				.layer_count = ktx_ptr->numLayers,
				.sample_count = VK_SAMPLE_COUNT_1_BIT,
				.tiling = VK_IMAGE_TILING_OPTIMAL,
				.usage = usage,
				.mip_offsets = mip_offsets
			},
			sl::move(bytes),
			data_size
		};
	}
}
