#pragma once
#include <memory>

#include "sirius/core/api.def.h"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/memory/image.hpp"
#include "sirius/arith/size.hpp"


namespace acma::vk {
    class SIRIUS_API depth_image : public image {
	public:
        static result<depth_image> create(
			std::shared_ptr<logical_device> const& logi_deivce,
			allocator_shared_handle const& allocator,
			extent2 extent
		) noexcept;
    };
}
