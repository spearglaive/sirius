#pragma once
#include <memory>

#include "sirius/core/api.def.h"
#include "sirius/core/make.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/memory/image.hpp"
#include "sirius/arith/size.hpp"


namespace acma::vk {
    class SIRIUS_API depth_image : public image {
	public:
		template<typename T>
		friend struct ::acma::impl::make;
    };
}

namespace acma::impl {
	template<>
    struct make<vk::depth_image> {
		SIRIUS_API result<vk::depth_image> operator()(
			sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
			sl::reference_ptr<const vk::logical_device> logi_device_ptr,
			sl::reference_ptr<const vk::allocator> allocator,
			extent2 extent,
			sl::in_place_adl_tag_type<vk::depth_image> = sl::in_place_adl_tag<vk::depth_image>
		) const noexcept;
	};
}