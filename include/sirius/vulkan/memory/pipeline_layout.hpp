#pragma once
#include <streamline/functional/functor/identity_index.hpp>

#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/api.def.h"
#include "sirius/vulkan/core/mixin.hpp"
#include "sirius/core/make.hpp"
#include "sirius/core/buffer_config_table.hpp"
#include "sirius/core/asset_heap_config_table.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/memory/asset_heap.hpp"
#include "sirius/vulkan/memory/bind_point.hpp"


namespace acma::vk {
    template<bind_point_t BindPoint, typename T, auto BufferConfigs, auto AssetHeapConfigs>
    struct pipeline_layout : mixin<VkPipelineLayout, PFN_vkDestroyPipelineLayout, logical_device> {
	public:
		template<typename>
		friend struct ::acma::impl::make;

	private:
		constexpr static sl::size_t max_push_descriptor_count = 32;
		
	private:
		descriptor_set_layout uniform_set_layout;
    };
}


namespace acma::impl {
    template<vk::bind_point_t BindPoint, typename T, auto BufferConfigs, auto AssetHeapConfigs>
    struct make<vk::pipeline_layout<BindPoint, T, BufferConfigs, AssetHeapConfigs>> {
		template<typename RenderProcessT>
		SIRIUS_API result<vk::pipeline_layout<BindPoint, T, BufferConfigs, AssetHeapConfigs>> operator()(
			sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
			sl::reference_ptr<const vk::logical_device> logi_device_ptr,
			RenderProcessT const& proc,
			sl::in_place_adl_tag_type<vk::pipeline_layout<BindPoint, T, BufferConfigs, AssetHeapConfigs>> = sl::in_place_adl_tag<vk::pipeline_layout<BindPoint, T, BufferConfigs, AssetHeapConfigs>>
		) const noexcept;
	};
}


#include "sirius/vulkan/memory/pipeline_layout.inl"