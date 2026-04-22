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


namespace acma::vk {
    template<shader_stage_flags_t Stages, typename T, auto BufferConfigs, auto AssetHeapConfigs>
    struct pipeline_layout : mixin<VkPipelineLayout, PFN_vkDestroyPipelineLayout, logical_device> {
	public:
		template<typename>
		friend struct ::acma::impl::make;
	private:
		using asset_heap_stage_filtered_sequence = sl::filtered_sequence_t<
			sl::index_sequence_of_length_type<decltype(T::asset_heaps)::size()>, 
			[]<sl::index_t I>(sl::index_constant_type<I>) noexcept {
				constexpr buffer_key_t key = sl::universal::get<I>(T::asset_heaps);
				return static_cast<bool>(AssetHeapConfigs[key].stages & Stages);
			}
		>;
		constexpr static sl::size_t asset_heap_count = asset_heap_stage_filtered_sequence::size();
		constexpr static sl::array<asset_heap_count, shader_stage_flags_t> set_layout_stages = 
			sl::universal::make<sl::array<asset_heap_count, shader_stage_flags_t>>(
				T::asset_heaps, 
				[](asset_heap_key_t key, auto) noexcept -> shader_stage_flags_t {
					return AssetHeapConfigs[key].stages;
				}
			
		);


	private:
		using uniform_stage_filtered_sequence = sl::filtered_sequence_t<
			sl::index_sequence_of_length_type<decltype(T::uniform_buffer_order)::size()>,
			[]<sl::index_t I>(sl::index_constant_type<I>) noexcept { 
				constexpr buffer_key_t key = sl::universal::get<I>(T::uniform_buffer_order);
				return static_cast<bool>(BufferConfigs[key].stages & Stages);
			}
		>;
		constexpr static sl::size_t uniform_buffer_count = uniform_stage_filtered_sequence::size();

	public:
		constexpr static sl::lookup_table<uniform_buffer_count, buffer_key_t, sl::uint32_t> uniform_buffer_binding_indices = 
			sl::universal::make<sl::lookup_table<uniform_buffer_count, buffer_key_t, sl::uint32_t>>(
				T::uniform_buffer_order,
				sl::functor::identity{}, sl::functor::identity_index{},
				uniform_stage_filtered_sequence{}
			);

	private:
		constexpr static sl::size_t max_push_descriptor_count = 32;
		
	private:
		descriptor_set_layout uniform_set_layout;
    };
}


namespace acma::impl {
    template<shader_stage_flags_t Stages, typename T, auto BufferConfigs, auto AssetHeapConfigs>
    struct make<vk::pipeline_layout<Stages, T, BufferConfigs, AssetHeapConfigs>> {
		template<typename RenderProcessT>
		SIRIUS_API result<vk::pipeline_layout<Stages, T, BufferConfigs, AssetHeapConfigs>> operator()(
			sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
			sl::reference_ptr<const vk::logical_device> logi_device_ptr,
			RenderProcessT const& proc,
			sl::in_place_adl_tag_type<vk::pipeline_layout<Stages, T, BufferConfigs, AssetHeapConfigs>> = sl::in_place_adl_tag<vk::pipeline_layout<Stages, T, BufferConfigs, AssetHeapConfigs>>
		) const noexcept;
	};
}


#include "sirius/vulkan/memory/pipeline_layout.inl"