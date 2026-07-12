#pragma once
#include "sirius/vulkan/display/image_view.hpp"

#include <streamline/algorithm/minmax.hpp>

#include "sirius/vulkan/memory/image.hpp"


namespace acma::vk {
	constexpr image_view	image_view::
	sublevels(sl::index_t pos, sl::size_t count) const noexcept {
		image_view ret{*this};
		ret._subresource.baseMipLevel = pos;
		ret._subresource.levelCount = sl::algo::min(count, ret._subresource.levelCount - pos);
		return ret;
	}


	constexpr image_view	image_view::
	sublayers(sl::index_t pos, sl::size_t count) const noexcept {
		image_view ret{*this};
		ret._subresource.baseArrayLayer = pos;
		ret._subresource.layerCount = sl::algo::min(count, ret._subresource.layerCount - pos);
		return ret;
	}
}


namespace acma::vk {
	constexpr 	image_view::
	operator VkImageViewCreateInfo() const noexcept {
		return {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = *_handle_ptr,
			.viewType = _subresource.layerCount > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D,
			.format = *_format_ptr,
			.components{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
			.subresourceRange = _subresource
		};
	}
}
