#include "sirius/vulkan/memory/image.hpp"

#include "sirius/core/make.hpp"
#include "sirius/core/memory_management.hpp"



namespace acma::impl {
	result<vk::image>
		make<vk::image>::
	operator()(
		sl::reference_ptr<const vk::allocator> allocator,
		vk::image_creation_info_t create_info,
		VkImageAspectFlags aspect_mask,
		bool dedicated_allocation,
		sl::in_place_adl_tag_type<vk::image>
	) const noexcept {
        vk::image ret;
        ret.aspect_flags = aspect_mask;
        ret.current_layout = sl::unique_ptr<VkImageLayout>(new VkImageLayout{VK_IMAGE_LAYOUT_UNDEFINED});
		RESULT_TRY_MOVE(ret.alloc_ptr, gpu_allocate(allocator, create_info, dedicated_allocation));
		return ret;
	};
}
