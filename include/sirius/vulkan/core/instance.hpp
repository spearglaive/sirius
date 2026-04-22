#pragma once

#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/api.def.h"
#include "sirius/vulkan/core/mixin.hpp"
#include "sirius/core/make.hpp"


namespace acma::vk {
	struct SIRIUS_API instance : mixin<VkInstance, PFN_vkDestroyInstance> {
	public:
		template<typename T>
		friend struct ::acma::impl::make;
	};
}


namespace acma::vk::impl {
	SIRIUS_API constexpr instance& vulkan_instance() noexcept {
		static instance i{};
		return i;
	}
}



namespace acma::impl {
	template<>
    struct make<vk::instance> {
		SIRIUS_API result<vk::instance> operator()(
			VkInstanceCreateInfo create_info,
			sl::in_place_adl_tag_type<vk::instance> = sl::in_place_adl_tag<vk::instance>
		) const noexcept;
	};
}