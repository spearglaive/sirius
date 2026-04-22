#pragma once
#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/api.def.h"
#include "sirius/vulkan/core/mixin.hpp"
#include "sirius/core/make.hpp"
#include "sirius/core/render_stage.hpp"
#include "sirius/vulkan/device/logical_device.hpp"


namespace acma::vk {
    struct SIRIUS_API semaphore : mixin<VkSemaphore, PFN_vkDestroySemaphore, logical_device> {
	public:
		template<typename T>
		friend struct ::acma::impl::make;
	public:
		result<void> wait(sl::uint64_t value, sl::uint64_t timeout = std::numeric_limits<sl::uint64_t>::max()) const noexcept;
	private:
		sl::reference_ptr<const vk::function_table> vulkan_fns_ptr;
		sl::reference_ptr<const vk::logical_device> logi_device_ptr;
    };
}

namespace acma::vk {
    struct semaphore_submit_info : public VkSemaphoreSubmitInfo {
        constexpr semaphore_submit_info() noexcept = default;
        constexpr semaphore_submit_info(VkSemaphore vk_semaphore, render_stage_flags_t stage_flags, std::uint64_t value = 0) noexcept :
            VkSemaphoreSubmitInfo{VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, nullptr, vk_semaphore, value, stage_flags, 0} {}
    };
}


namespace acma::impl {
	template<>
    struct make<vk::semaphore> {
		SIRIUS_API result<vk::semaphore> operator()(
			sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
			sl::reference_ptr<const vk::logical_device> logi_device_ptr,
			VkSemaphoreType semaphore_type = VK_SEMAPHORE_TYPE_BINARY,
			sl::in_place_adl_tag_type<vk::semaphore> = sl::in_place_adl_tag<vk::semaphore>
		) const noexcept;
	};
}