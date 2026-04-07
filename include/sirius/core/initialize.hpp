#pragma once
#include <memory>
#include <set>

#include "sirius/core/api.def.h"
#include "sirius/core/version.hpp"
#include "sirius/core/error.hpp"
#include "sirius/vulkan/device/physical_device.hpp"


#ifndef SIRIUS_WINDOWING_CAPABILITIES
#define SIRIUS_WINDOWING_CAPABILITIES true
#endif

namespace acma::impl {
	constexpr bool window_capability = SIRIUS_WINDOWING_CAPABILITIES;
}


namespace acma {
	SIRIUS_API result<void> intitialize_lib(std::string_view app_name, version app_version) noexcept;
	SIRIUS_API void terminate_lib() noexcept;
}

namespace acma {
	SIRIUS_API std::vector<vk::physical_device>& devices() noexcept;
}


namespace acma::impl {
	SIRIUS_API std::string& name() noexcept;
}
