#pragma once
#include <streamline/memory/reference_ptr.hpp>
#include <streamline/numeric/int.hpp>

#include "sirius/vulkan/core/vulkan.hpp"
#include "sirius/core/api.def.h"


namespace acma::vk{
	struct SIRIUS_API image;
}


namespace acma::vk {
	struct image_view {
		sl::reference_ptr<const VkFormat> _format_ptr;
		sl::reference_ptr<const VkImageLayout> _layout_ptr;
		sl::reference_ptr<const VkImage> _handle_ptr;
		VkImageSubresourceRange _subresource;
	public:
		constexpr VkFormat      format() const noexcept { return *_format_ptr; }
		constexpr VkImageLayout layout() const noexcept { return *_layout_ptr; }
		constexpr VkImage       handle() const noexcept { return *_handle_ptr; }
	public:
		constexpr sl::uint32_t first_level() const noexcept { return _subresource.baseMipLevel; }
		constexpr sl::uint32_t level_count() const noexcept { return _subresource.levelCount; }
		constexpr sl::uint32_t first_layer() const noexcept { return _subresource.baseArrayLayer; }
		constexpr sl::uint32_t layer_count() const noexcept { return _subresource.layerCount; }

	public:
		constexpr image_view sublevels(sl::index_t pos, sl::size_t count = sl::dynamic_extent) const noexcept;
		constexpr image_view sublayers(sl::index_t pos, sl::size_t count = sl::dynamic_extent) const noexcept;

	public:
		constexpr operator VkImageViewCreateInfo() const noexcept;
	};
}


#include "sirius/vulkan/display/image_view.inl"
