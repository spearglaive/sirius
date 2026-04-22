#pragma once
#include <span>

#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/api.def.h"
#include "sirius/vulkan/core/mixin.hpp"
#include "sirius/core/make.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/memory/bind_point.hpp"
#include "sirius/vulkan/memory/pipeline_layout.hpp"

namespace acma::vk {
	template<bind_point_t BindPoint, typename T, auto BufferConfigs, auto AssetHeapConfigs>
    struct pipeline;
}


namespace acma::vk {
    template<typename T, auto BufferConfigs, auto AssetHeapConfigs>
    struct pipeline<bind_point::graphics, T, BufferConfigs, AssetHeapConfigs> : mixin<VkPipeline, PFN_vkDestroyPipeline, logical_device> {
	public:
		template<typename>
		friend struct ::acma::impl::make;
	public:
		constexpr auto&& layout(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._layout); }

	private:
		pipeline_layout<bind_point::graphics, T, BufferConfigs, AssetHeapConfigs> _layout;
    };


    template<typename T, auto BufferConfigs, auto AssetHeapConfigs>
    struct pipeline<bind_point::compute, T, BufferConfigs, AssetHeapConfigs> : mixin<VkPipeline, PFN_vkDestroyPipeline, logical_device> {
	public:
		template<typename>
		friend struct ::acma::impl::make;
	public:
		constexpr auto&& layout(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._layout); }

	private:
		pipeline_layout<bind_point::compute, T, BufferConfigs, AssetHeapConfigs> _layout;
    };
}



namespace acma::impl {
    template<typename T, auto BufferConfigs, auto AssetHeapConfigs>
    struct make<vk::pipeline<vk::bind_point::graphics, T, BufferConfigs, AssetHeapConfigs>> {
		template<typename RenderProcessT>
		SIRIUS_API result<vk::pipeline<vk::bind_point::graphics, T, BufferConfigs, AssetHeapConfigs>> operator()(
			sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
			sl::reference_ptr<const vk::logical_device> logi_device_ptr,
			RenderProcessT&& proc,
			std::span<const VkFormat> color_attachment_formats,
			VkFormat depth_attachment_format,
			sl::in_place_adl_tag_type<vk::pipeline<vk::bind_point::graphics, T, BufferConfigs, AssetHeapConfigs>> = sl::in_place_adl_tag<vk::pipeline<vk::bind_point::graphics, T, BufferConfigs, AssetHeapConfigs>>
		) const noexcept;
	};
	
    template<typename T, auto BufferConfigs, auto AssetHeapConfigs>
    struct make<vk::pipeline<vk::bind_point::compute, T, BufferConfigs, AssetHeapConfigs>> {
		template<typename RenderProcessT>
		SIRIUS_API result<vk::pipeline<vk::bind_point::compute, T, BufferConfigs, AssetHeapConfigs>> operator()(
			sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
			sl::reference_ptr<const vk::logical_device> logi_device_ptr,
			RenderProcessT&& proc,
			sl::in_place_adl_tag_type<vk::pipeline<vk::bind_point::compute, T, BufferConfigs, AssetHeapConfigs>> = sl::in_place_adl_tag<vk::pipeline<vk::bind_point::compute, T, BufferConfigs, AssetHeapConfigs>>
		) const noexcept;
	};
}



#include "sirius/vulkan/memory/pipeline.inl"