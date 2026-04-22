#pragma once
#include <string_view>
#include <compare>
#include "sirius/vulkan/core/vulkan.hpp"
#include "sirius/vulkan/core/vulkan.hpp"
#include <streamline/containers/array.hpp>

#include "sirius/core/make.hpp"
#include "sirius/vulkan/core/unique_vk_ptr.hpp"
#include "sirius/vulkan/core/mixin.hpp"
#include "sirius/core/api.def.h"
#include "sirius/core/command_family.hpp"
#include "sirius/core/error.hpp"
#include "sirius/vulkan/device/device_query.hpp"
#include "sirius/vulkan/device/device_query_traits.hpp"
#include "sirius/vulkan/device/queue_family_info.hpp"
#include "sirius/vulkan/device/extension.hpp"
#include "sirius/vulkan/device/feature.hpp"
#include "sirius/vulkan/device/device_type.hpp"
#include "sirius/vulkan/display/surface.hpp"
#include "sirius/vulkan/memory/asset_group.hpp"
#include "sirius/vulkan/memory/descriptor_heap.hpp"
#include "sirius/vulkan/memory/asset_usage_policy.hpp"


namespace acma::vk {
	struct asset_group_info {
		sl::size_t size;
		sl::size_t alignment;
	};

	struct descriptor_heap_info {
		sl::size_t minimum_reserved_bytes;
		sl::size_t alignment;
		sl::size_t max_size;
	};
}

namespace acma::vk::impl {
	constexpr sl::uint32_t nidx = (static_cast<std::uint32_t>(sl::npos) >> 1);
}

namespace acma::vk {
    struct physical_device {
        SIRIUS_API static result<physical_device> create(VkPhysicalDevice& device_handle) noexcept;

		SIRIUS_API result<void> initialize_queues(bool prefer_synchronous_rendering, bool window_capability) noexcept;

		template<typename T>
		friend struct ::acma::impl::make;

    public:
        template<device_query Query> typename device_query_traits<Query>::return_type query(surface const& s) const noexcept = delete;

        template<> SIRIUS_API typename device_query_traits<device_query::surface_capabilites>::return_type query<device_query::surface_capabilites>(surface const& s) const noexcept;
        template<> SIRIUS_API typename device_query_traits<device_query::display_formats    >::return_type query<device_query::display_formats    >(surface const& s) const noexcept;
        template<> SIRIUS_API typename device_query_traits<device_query::present_modes      >::return_type query<device_query::present_modes      >(surface const& s) const noexcept;

	public:
		constexpr operator VkPhysicalDevice      &()      & noexcept { return handle; }
		constexpr operator VkPhysicalDevice const&() const& noexcept { return handle; }
        
    public:
		VkPhysicalDevice handle;
        std::string_view name;
        device_type type = device_type::unknown;
        extensions_t extensions{};
        features_t features{};
        VkPhysicalDeviceLimits limits{};
		sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> descriptor_count_limits{};
		sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> per_stage_descriptor_count_limits{};

		sl::size_t max_push_data_bytes;
		sl::array<asset_group::num_asset_groups, asset_group_info> asset_group_infos{};
		sl::array<descriptor_heap::num_descriptor_heaps, descriptor_heap_info> descriptor_heap_infos{};
        sl::array<command_family::num_families, queue_family_info> queue_family_infos{};
		sl::uint32_t max_queue_count;

    public:
        constexpr friend std::strong_ordering operator<=>(const physical_device& a, const physical_device& b) noexcept;

    };
}


namespace acma::vk {
    constexpr std::strong_ordering operator<=>(const physical_device& a, const physical_device& b) noexcept {
        std::int32_t a_type_rating = a.type == device_type::discrete_gpu ? -1 : static_cast<std::int32_t>(a.type);
        std::int32_t b_type_rating = b.type == device_type::discrete_gpu ? -1 : static_cast<std::int32_t>(b.type);
        return a_type_rating <=> b_type_rating;
    }
}



namespace acma::impl {
	template<>
    struct make<vk::physical_device> {
		SIRIUS_API result<vk::physical_device> operator()(
			VkPhysicalDevice device_handle,
			sl::in_place_adl_tag_type<vk::physical_device> = sl::in_place_adl_tag<vk::physical_device>
		) const noexcept;
	};
}
