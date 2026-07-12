#pragma once
#include <memory>

#include "sirius/core/make.hpp"
#include "sirius/graphics/core/texture_info.hpp"
#include "sirius/vulkan/core/mixin.hpp"
#include "sirius/vulkan/core/unique_vk_ptr.hpp"
#include "sirius/arith/point.hpp"
#include "sirius/arith/size.hpp"
#include "sirius/core/descriptor_key_t.hpp"
#include "sirius/core/buffer_key_t.hpp"
#include "sirius/core/error.hpp"
#include "sirius/vulkan/core/vulkan.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/memory/image_allocation.hpp"
#include "sirius/vulkan/memory/resizable_gpu_buffer.fwd.hpp"
#include "sirius/core/memory_management.fwd.hpp"
#include "sirius/vulkan/memory/allocator.hpp"


// __D2D_DECLARE_VK_TRAITS_DEVICE(VkImage);
namespace acma::vk {
	struct image_view;
}


namespace acma::vk {
	struct SIRIUS_API image {
	public:
		template<typename T>
		friend struct ::acma::impl::make;

	public:
		constexpr sl::reference_ptr<const VkImage>           handle_ref()        const& noexcept { return {std::addressof(alloc_ptr->handle)}; }
		constexpr sl::reference_ptr<const VkImageCreateInfo> creation_info_ptr() const& noexcept { return {&alloc_ptr->creation_info}; }
		constexpr sl::reference_ptr<const VkImageLayout>     layout_ptr()        const& noexcept { return current_layout; }
		constexpr sl::reference_ptr<      VkImageLayout>     layout_ptr()             & noexcept { return current_layout; }

		constexpr sl::size_t dimensions() const noexcept { return static_cast<sl::size_t>(alloc_ptr->creation_info.imageType) + 1; }


		constexpr VkFormat              format_id()       const noexcept { return alloc_ptr->creation_info.format; }
		constexpr VkExtent3D            size()            const noexcept { return alloc_ptr->creation_info.extent; }
		constexpr sl::uint32_t          mip_level_count() const noexcept { return alloc_ptr->creation_info.mipLevels; }
		constexpr sl::uint32_t          layer_count()     const noexcept { return alloc_ptr->creation_info.arrayLayers; }
		constexpr VkSampleCountFlagBits sample_count()    const noexcept { return alloc_ptr->creation_info.samples; }
		constexpr VkImageTiling         tiling()          const noexcept { return alloc_ptr->creation_info.tiling; }
		constexpr VkImageUsageFlags     usage()           const noexcept { return alloc_ptr->creation_info.usage; }

		constexpr sl::size_t            size_bytes()      const noexcept { return alloc_ptr->allocation_info.size; }

		constexpr VkImageLayout      layout()  const noexcept { return *current_layout; }
		constexpr VkImageAspectFlags aspects() const noexcept { return aspect_flags; }

		constexpr operator image_view() const noexcept;

	protected:
		image_allocation_unique_ptr alloc_ptr;
		sl::unique_ptr<VkImageLayout> current_layout;
		VkImageAspectFlags aspect_flags;
	};
}


namespace acma::impl {
	template<>
	struct make<vk::image> {
		SIRIUS_API result<vk::image> operator()(
			sl::reference_ptr<const vk::allocator> allocator,
			vk::image_creation_info_t create_info,
			VkImageAspectFlags image_aspects = VK_IMAGE_ASPECT_COLOR_BIT,
			bool dedicated_allocation = false,
			sl::in_place_adl_tag_type<vk::image> = sl::in_place_adl_tag<vk::image>
		) const noexcept;
	};
}



#include "sirius/vulkan/display/image_view.hpp"
namespace acma::vk {
	constexpr    image::
	operator image_view() const noexcept {
		return {
			._format_ptr{&creation_info_ptr()->format},
			._layout_ptr = layout_ptr(),
			._handle_ptr = handle_ref(),
			._subresource{
				.aspectMask = aspects(),
				.baseMipLevel = 0,
				.levelCount = mip_level_count(),
				.baseArrayLayer = 0,
				.layerCount = layer_count(),
			},
		};
	}
}
