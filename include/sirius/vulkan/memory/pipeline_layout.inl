#pragma once
#include "sirius/core/buffer_info.hpp"
#include "sirius/vulkan/memory/pipeline_layout.hpp"

#include <streamline/functional/functor/forward_construct.hpp>
#include <streamline/functional/functor/invoke_each.hpp>
#include <streamline/metaprogramming/integer_sequence.hpp>


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
			[](buffer_info info, auto) noexcept -> VkPushConstantRange {
				const buffer_config cfg = BufferConfigs[info.buffer_key];
				const sl::size_t data_size = (info.size == VK_WHOLE_SIZE ? cfg.initial_capacity_bytes : info.size);
				return {cfg.stages, 0, static_cast<std::uint32_t>(data_size)};
			}
		);


		constexpr static auto uniform_bindings = sl::make<sl::array<T::uniform_infos.size(), VkDescriptorSetLayoutBinding>>(
			T::uniform_infos,
			[]<sl::index_t I>(buffer_info info, sl::index_constant_type<I>) noexcept {
				return VkDescriptorSetLayoutBinding{
					.binding = I,
					.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.descriptorCount = 1,
					.stageFlags = BufferConfigs[info.buffer_key].stages,
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


		constexpr sl::size_t descriptor_set_count = T::asset_heaps.size();
		sl::array<1 + descriptor_set_count, VkDescriptorSetLayout> set_layout_handles{};
		set_layout_handles[0] = ret.uniform_set_layout;

		constexpr auto get_set_layout = []<sl::index_t I>(
			RenderProcessT const& render_proc,
			decltype(set_layout_handles)& out,
			sl::index_constant_type<I>
		) noexcept {
			out[I + 1] = sl::universal::get<T::asset_heaps[I]>(render_proc).set_layout();
		};

		sl::invoke(
			sl::functor::invoke_each<get_set_layout>{},
			sl::index_sequence_of_length<descriptor_set_count>,
			proc,
			set_layout_handles
		);


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
