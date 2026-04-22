#pragma once
#include "sirius/vulkan/memory/pipeline_layout.hpp"

#include <streamline/functional/functor/forward_construct.hpp>
#include <streamline/functional/functor/invoke_each.hpp>
#include <streamline/metaprogramming/integer_sequence.hpp>

#include "sirius/core/push_constant_buffer_info.hpp"


namespace acma::impl {
    template<vk::bind_point_t BindPoint, typename T, auto BufferConfigs, auto AssetHeapConfigs>
	template<typename RenderProcessT>
	result<vk::pipeline_layout<BindPoint, T, BufferConfigs, AssetHeapConfigs>>
		make<vk::pipeline_layout<BindPoint, T, BufferConfigs, AssetHeapConfigs>>::
	operator()(
		sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
		sl::reference_ptr<const vk::logical_device> logi_device_ptr,
		RenderProcessT const& proc,
		sl::in_place_adl_tag_type<vk::pipeline_layout<BindPoint, T, BufferConfigs, AssetHeapConfigs>>
	) const noexcept {
		using pipeline_layout = vk::pipeline_layout<BindPoint, T, BufferConfigs, AssetHeapConfigs>;

		pipeline_layout ret{};
		ret.smart_handle = {vulkan_fns_ptr->vkDestroyPipelineLayout, logi_device_ptr};

		constexpr static auto push_constant_ranges = sl::make<sl::array<T::push_constant_infos.size(), VkPushConstantRange>>(
			T::push_constant_infos,
			[](push_constant_buffer_info info, auto) noexcept -> VkPushConstantRange {
				const buffer_config cfg = BufferConfigs[info.buffer_key];
				return {cfg.stages, static_cast<sl::uint32_t>(info.offset), static_cast<std::uint32_t>(cfg.initial_capacity_bytes)};
			}
		);


		constexpr static auto uniform_bindings = sl::make<sl::array<T::uniform_buffers.size(), VkDescriptorSetLayoutBinding>>(
			T::uniform_buffers,
			[]<sl::index_t I>(buffer_key_t key, sl::index_constant_type<I>) noexcept {
				return VkDescriptorSetLayoutBinding{
					.binding = I,
					.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.descriptorCount = 1,
					.stageFlags = BufferConfigs[key].stages,
					.pImmutableSamplers = nullptr,
				};
			}
		);
		constexpr static VkDescriptorSetLayoutCreateInfo set_layout_info{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT,
			.bindingCount = uniform_bindings.size(),
			.pBindings = uniform_bindings.data(),
		};

		RESULT_TRY_MOVE(ret.uniform_set_layout, acma::make<vk::descriptor_set_layout>(
			vulkan_fns_ptr,
			logi_device_ptr,
			set_layout_info
		));


		constexpr bool has_asset_heap = requires{ T::asset_heap; };
		constexpr sl::size_t descriptor_set_count = has_asset_heap ? asset_usage_policy::num_usage_policies : 0;
		sl::array<1 + descriptor_set_count, VkDescriptorSetLayout> set_layout_handles{};
		set_layout_handles[0] = ret.uniform_set_layout;
		if constexpr(has_asset_heap)
			for(asset_usage_policy_t j = 0; j < descriptor_set_count; ++j) 
				set_layout_handles[1 + j] = sl::universal::get<T::asset_heap>(proc)._descriptor_set_layouts[j];
		
		
		const VkPipelineLayoutCreateInfo pipeline_layout_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = set_layout_handles.size(),
			.pSetLayouts = set_layout_handles.data(),
			.pushConstantRangeCount = push_constant_ranges.size(),
			.pPushConstantRanges = push_constant_ranges.data(),
        };
        __D2D_VULKAN_VERIFY(sl::invoke(vulkan_fns_ptr->vkCreatePipelineLayout, *logi_device_ptr, &pipeline_layout_create_info, nullptr, &ret));

        return ret;
	}
}