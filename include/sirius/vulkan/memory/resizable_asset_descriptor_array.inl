#pragma once
#include "sirius/vulkan/memory/resizable_asset_descriptor_array.hpp"


namespace acma::vk::impl {
	template<descriptor_array_config Config, sl::size_t CommandGroupCount>
	constexpr result<void>
		resizable_asset_descriptor_array_base<Config, CommandGroupCount>::
	initialize() noexcept {
		constexpr static sl::uint32_t stage_count = std::popcount(config.stages);
		const sl::uint32_t max_descriptor_count = std::min(std::min(
			render_proc_core().physical_device_ptr()->descriptor_count_limits[config.usage] / stage_count,
			render_proc_core().physical_device_ptr()->per_stage_descriptor_count_limits[config.usage]),
			render_proc_core().physical_device_ptr()->limits.maxPerStageResources
		);

		//Make set layout
		{
		const VkDescriptorSetLayoutBinding set_layout_binding{
			.binding = 0,
			.descriptorType = descriptor_type,
			.descriptorCount = max_descriptor_count,
			.stageFlags = config.stages,
			.pImmutableSamplers = nullptr,
		};
		constexpr static VkDescriptorBindingFlags binding_flags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
		const VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags_info{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
			.pNext = nullptr,
			.bindingCount = 1,
			.pBindingFlags = &binding_flags
		};
		const VkDescriptorSetLayoutCreateInfo set_layout_info{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = &binding_flags_info,
			.flags = VkDescriptorSetLayoutCreateFlags{},
			.bindingCount = 1,
			.pBindings = &set_layout_binding,
		};

		RESULT_TRY_MOVE(_descriptor_set_layout, acma::make<descriptor_set_layout>(
			render_proc_core().vulkan_functions_ptr(),
			render_proc_core().logical_device_ptr(),
			set_layout_info
		));
		}

		return {};
	}
}

namespace acma::vk::impl {
	template<descriptor_array_config Config, sl::size_t CommandGroupCount>
	template<typename DescriptorT>
	constexpr result<void>
		resizable_asset_descriptor_array_base<Config, CommandGroupCount>::
	update_descriptors(
		std::vector<DescriptorT> const& descriptor_assets
	) noexcept {
		const sl::uint32_t new_descriptor_count = static_cast<sl::uint32_t>(descriptor_assets.size());

		//Some graphics drivers are bugged and tweak out when you pass 0 as the descriptor count
		if(new_descriptor_count == 0) return {};

		if(new_descriptor_count <= _descriptor_capacity)
			goto write_descriptors;


		//Make pool
		{
		const VkDescriptorPoolSize pool_size{
			.type = descriptor_type,
			.descriptorCount = new_descriptor_count
		};

		const VkDescriptorPoolCreateInfo pool_create_info{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VkDescriptorPoolCreateFlags{},
			.maxSets = 1,
			.poolSizeCount = 1,
			.pPoolSizes = &pool_size,
		};

		RESULT_TRY_MOVE(_descriptor_pool, acma::make<descriptor_pool>(
			render_proc_core().vulkan_functions_ptr(),
			render_proc_core().logical_device_ptr(),
			pool_create_info
		));
		}


		//Make set
		{
		const VkDescriptorSetVariableDescriptorCountAllocateInfo variable_count_alloc_info{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorSetCount = 1,
			.pDescriptorCounts = &new_descriptor_count
		};
		const VkDescriptorSetAllocateInfo set_alloc_info{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = &variable_count_alloc_info,
			.descriptorPool = _descriptor_pool,
			.descriptorSetCount = 1,
			.pSetLayouts = &_descriptor_set_layout
		};
		RESULT_TRY_MOVE(_descriptor_set, acma::make<descriptor_set>(
			render_proc_core().vulkan_functions_ptr(),
			render_proc_core().logical_device_ptr(),
			//_descriptor_pool_ptrs[alloc_idx],
			set_alloc_info
		));
		}


		//Finalize
		_descriptor_capacity = new_descriptor_count;


	write_descriptors:
		//Generate descriptors
		const std::unique_ptr<VkDescriptorImageInfo[]> descriptor_infos = std::make_unique_for_overwrite<VkDescriptorImageInfo[]>(new_descriptor_count);
		for(sl::index_t i = 0; i < new_descriptor_count; ++i) {
			new (&descriptor_infos[i]) VkDescriptorImageInfo(descriptor_assets[i]);
		}

		//Write descriptors
		{
		const VkWriteDescriptorSet write{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = _descriptor_set,
			.dstBinding = 0,
			.descriptorCount = new_descriptor_count,
			.descriptorType = descriptor_type,
			.pImageInfo = descriptor_infos.get(),
		};
		_descriptor_set.update({&write, 1});
		}

		return {};
	}
}


namespace acma::vk {
	template<descriptor_array_config Config, typename RenderProcessCoreT>
	requires (Config.usage != asset_usage_policy::sampler)
	constexpr result<void>
		resizable_asset_descriptor_array<Config, RenderProcessCoreT>::
	emplace_back(image_view view) noexcept {
		result<image_view_handle> image_view_handle_result = ::acma::make<image_view_handle>(
			this->render_proc_core().vulkan_functions_ptr(),
			this->render_proc_core().logical_device_ptr(),
			view
		);
		if(!image_view_handle_result.has_value())
			return image_view_handle_result.error();
		this->push_back(*sl::move(image_view_handle_result));

		return this->update_descriptors(*this);
	}
}

namespace acma::vk {
	template<descriptor_array_config Config, typename RenderProcessCoreT>
	requires (Config.usage == asset_usage_policy::sampler)
	constexpr result<void>
		resizable_asset_descriptor_array<Config, RenderProcessCoreT>::
	emplace_back(VkSamplerCreateInfo info) noexcept {
		result<image_sampler> image_sampler_result = ::acma::make<image_sampler>(
			this->render_proc_core().vulkan_functions_ptr(),
			this->render_proc_core().logical_device_ptr(),
			static_cast<VkSamplerCreateInfo>(info)
		);
		if(!image_sampler_result.has_value())
			return image_sampler_result.error();
		this->push_back(*sl::move(image_sampler_result));

		return this->update_descriptors(*this);
	}
}
