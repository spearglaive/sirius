#include "sirius/vulkan/display/image_sampler.hpp"



namespace acma::impl {
	result<vk::image_sampler>
		make<vk::image_sampler>::
	operator()(
		sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
		sl::reference_ptr<const vk::logical_device> logi_device_ptr,
		float max_anisotropy,
		pt3<VkSamplerAddressMode> address_modes,
		sl::in_place_adl_tag_type<vk::image_sampler>
	) const noexcept {
		const VkSamplerCreateInfo sampler_create_info {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
	        .magFilter               = VK_FILTER_LINEAR,
	        .minFilter               = VK_FILTER_LINEAR,
	        .mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST,
	        .addressModeU            = address_modes.x(),
	        .addressModeV            = address_modes.y(),
	        .addressModeW            = address_modes.z(),
            .mipLodBias              = 0.0f,
            .anisotropyEnable        = VK_TRUE,
            .maxAnisotropy           = max_anisotropy,
            .compareEnable           = VK_FALSE,
            .compareOp               = VK_COMPARE_OP_ALWAYS,
            .minLod                  = 0.0f,
	        .maxLod                  = VK_LOD_CLAMP_NONE,
            .borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
        };

        return sl::invoke(*this, vulkan_fns_ptr, logi_device_ptr, sampler_create_info);
	}
}



namespace acma::impl {
	result<vk::image_sampler>
		make<vk::image_sampler>::
	operator()(
		sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
		sl::reference_ptr<const vk::logical_device> logi_device_ptr,
		VkSamplerCreateInfo create_info,
		sl::in_place_adl_tag_type<vk::image_sampler>
	) const noexcept {
		vk::image_sampler ret{
			{{vulkan_fns_ptr->vkDestroySampler, logi_device_ptr}},
			{{{create_info.addressModeU, create_info.addressModeV, create_info.addressModeW}}}
		};

        __D2D_VULKAN_VERIFY(sl::invoke(vulkan_fns_ptr->vkCreateSampler, *logi_device_ptr, &create_info, nullptr, &ret));
        return ret;
	}
}
