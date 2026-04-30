#pragma once
#include "sirius/vulkan/memory/asset_heap.hpp"

#include <streamline/algorithm/aligned_to.hpp>
#include <streamline/functional/functor/forward_construct.hpp>

#include "sirius/vulkan/sync/semaphore.hpp"
#include "sirius/timeline/dedicated_command_group.hpp"
#include "sirius/core/memory_management.hpp"



namespace acma::vk::impl {
	template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
	constexpr result<void>
		asset_heap_base<K, AssetHeapConfigs, RenderProcessT>::
	initialize() noexcept {
		RenderProcessT const& proc = static_cast<RenderProcessT const&>(*this);

		constexpr static sl::uint32_t stage_count = std::popcount(config.stages);
		const sl::uint32_t max_descriptor_count = std::min(std::min(
			proc.physical_device_ptr()->descriptor_count_limits[config.usage] / stage_count,
			proc.physical_device_ptr()->per_stage_descriptor_count_limits[config.usage]),
			proc.physical_device_ptr()->limits.maxPerStageResources
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
			proc.vulkan_functions_ptr(),
			proc.logical_device_ptr(),
			set_layout_info
		));
		}

		return {};
	}
}

namespace acma::vk::impl {
	template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
	constexpr result<void>
		asset_heap_base<K, AssetHeapConfigs, RenderProcessT>::
	update_descriptors(
		std::span<const VkDescriptorImageInfo> infos
	) noexcept {
		const sl::uint32_t new_total_count = static_cast<sl::uint32_t>(infos.size());

		//Some graphics drivers are bugged and tweak out when you pass 0 as the descriptor count
		if(new_total_count == 0) return {};

		RenderProcessT const& proc = static_cast<RenderProcessT const&>(*this);
		const sl::index_t alloc_idx = this->allocation_index();

		if(new_total_count <= _descriptor_capacities[alloc_idx])
			goto write_descriptors;


		//Make pool
		{
		const VkDescriptorPoolSize pool_size{
			.type = descriptor_type,
			.descriptorCount = new_total_count
		};

		const VkDescriptorPoolCreateInfo pool_create_info{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VkDescriptorPoolCreateFlags{},
			.maxSets = allocation_count,
			.poolSizeCount = 1,
			.pPoolSizes = &pool_size,
		};

		RESULT_TRY_MOVE(_descriptor_pools[alloc_idx], acma::make<descriptor_pool>(
			proc.vulkan_functions_ptr(),
			proc.logical_device_ptr(),
			pool_create_info
		));
		}


		//Make set
		{
		const VkDescriptorSetVariableDescriptorCountAllocateInfo variable_count_alloc_info{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorSetCount = 1,
			.pDescriptorCounts = &new_total_count
		};
		const VkDescriptorSetAllocateInfo set_alloc_info{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = &variable_count_alloc_info,
			.descriptorPool = _descriptor_pools[alloc_idx],
			.descriptorSetCount = 1,
			.pSetLayouts = &_descriptor_set_layout
		};
		RESULT_TRY_MOVE(_descriptor_sets[alloc_idx], acma::make<descriptor_set>(
			proc.vulkan_functions_ptr(),
			proc.logical_device_ptr(),
			//_descriptor_pool_ptrs[alloc_idx],
			set_alloc_info
		));
		}


		//Finalize
		_descriptor_capacities[alloc_idx] = new_total_count;


	write_descriptors:
		//Write descriptors
		{
		const VkWriteDescriptorSet write{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = _descriptor_sets[alloc_idx],
			.dstBinding = 0,
			.descriptorCount = new_total_count,
			.descriptorType = descriptor_type,
			.pImageInfo = infos.data(),
		};
		_descriptor_sets[alloc_idx].update({&write, 1});
		}

		return {};
	}
}


namespace acma::vk {
	template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
	template<buffer_key_t BufferKey, auto BufferConfigs>
	constexpr result<void>   asset_heap<K, AssetHeapConfigs, RenderProcessT>::
 	emplace_back(buffer<BufferKey, BufferConfigs, RenderProcessT> const& texture_data_buffer) noexcept {
  		static_assert((buffer<BufferKey, BufferConfigs, RenderProcessT>::config.usage & buffer_usage_policy::texture_data) == buffer_usage_policy::texture_data);

		RenderProcessT const& proc = static_cast<RenderProcessT const&>(*this);
		if(!this->last_clear_frame.matches(proc.frame_count()))
			return errc::buffer_needs_changes_applied; //TODO

		if(texture_data_buffer.texture_data_infos.empty() || texture_data_buffer.size() == 0)
			return {};


		const sl::size_t old_image_count = this->size();
		const sl::size_t new_descriptor_count = texture_data_buffer.texture_data_infos.size();

		RESULT_VERIFY(make_images(texture_data_buffer.texture_data_infos));

		RESULT_VERIFY(gpu_copy({this->data() + old_image_count, new_descriptor_count}, texture_data_buffer));

		for(sl::index_t i = 0; i < new_descriptor_count; ++i) {
			const asset_usage_policy_t texture_usage = texture_data_buffer.texture_data_infos[i].usage == texture_usage::sampled ?
				asset_usage_policy::sampled_image :
				asset_usage_policy::storage_image;
			if(texture_usage != config.usage)
				return errc::invalid_argument;
		}

		const sl::index_t alloc_idx = this->allocation_index();

		const sl::size_t total_descriptor_count = this->size();
		const std::unique_ptr<VkDescriptorImageInfo[]> descriptor_infos = std::make_unique_for_overwrite<VkDescriptorImageInfo[]>(total_descriptor_count);
		for(sl::index_t i = 0; i < total_descriptor_count; ++i) {
			new (&descriptor_infos[i]) VkDescriptorImageInfo{
				.imageView{*_images[alloc_idx][i].view_ref()},
				.imageLayout{_images[alloc_idx][i].current_layout}
			};
		}
		return this->update_descriptors({descriptor_infos.get(), total_descriptor_count});
	}
}

namespace acma::vk {
	template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
	constexpr void   asset_heap<K, AssetHeapConfigs, RenderProcessT>::
	clear() noexcept {
		_images[this->allocation_index()].clear();

		RenderProcessT const& proc = static_cast<RenderProcessT const&>(*this);
		this->last_clear_frame = proc.frame_count();
	}
}


namespace acma::vk {
	template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
	constexpr result<void>
		asset_heap<K, AssetHeapConfigs, RenderProcessT>::
	make_images(std::vector<texture_data_info> const& texture_data_infos) noexcept {
		RenderProcessT const& proc = static_cast<RenderProcessT const&>(*this);
		const sl::index_t alloc_idx = this->allocation_index();

		_images[alloc_idx].reserve(this->size() + texture_data_infos.size());
		for(sl::size_t i = 0; i < texture_data_infos.size(); ++i) {
			RESULT_VERIFY_UNSCOPED(make<image>(
				proc.vulkan_functions_ptr(),
				proc.logical_device_ptr(),
				proc.allocator_ptr(),
				static_cast<VkImageCreateInfo>(texture_data_infos[i])
			), img_result);
			_images[alloc_idx].push_back(*sl::move(img_result));
		}

		return {};
	}
}



namespace acma::vk {
	template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
	requires(AssetHeapConfigs[K].usage == asset_usage_policy::sampler)
	template<typename T>
	constexpr result<void>    asset_heap<K, AssetHeapConfigs, RenderProcessT>::
	push_back(T&& t)
	noexcept(sl::traits::is_noexcept_constructible_from_v<VkSamplerCreateInfo, T&&>)
	requires(sl::traits::is_constructible_from_v<VkSamplerCreateInfo, T&&>) {
		RenderProcessT const& proc = static_cast<RenderProcessT const&>(*this);
		if(!this->last_clear_frame.matches(proc.frame_count()))
			return errc::buffer_needs_changes_applied; //TODO


		const sl::index_t alloc_idx = this->allocation_index();

		_sampler_infos[alloc_idx].push_back(sl::forward<T>(t));


		result<image_sampler> sampler_result = acma::make<image_sampler>(
			proc.vulkan_functions_ptr(),
			proc.logical_device_ptr(),
			_sampler_infos[alloc_idx].back()
		);
		if(!sampler_result.has_value()) [[unlikely]] {
			_sampler_infos[alloc_idx].pop_back();
			return sampler_result.error();
		}
		_samplers[alloc_idx].push_back(*sl::move(sampler_result));

		const sl::size_t total_descriptor_count = this->size();
		const std::unique_ptr<VkDescriptorImageInfo[]> descriptor_infos = std::make_unique_for_overwrite<VkDescriptorImageInfo[]>(total_descriptor_count);
		for(sl::index_t i = 0; i < total_descriptor_count; ++i) {
			new (&descriptor_infos[i]) VkDescriptorImageInfo{
				.sampler{_samplers[alloc_idx][i]}
			};
		}
		return this->update_descriptors({descriptor_infos.get(), total_descriptor_count});
	}

	template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
	requires(AssetHeapConfigs[K].usage == asset_usage_policy::sampler)
	template<typename... Args>
	constexpr result<void>    asset_heap<K, AssetHeapConfigs, RenderProcessT>::
	emplace_back(Args&&... args)
	noexcept(sl::traits::is_noexcept_constructible_from_v<VkSamplerCreateInfo, Args&&...>)
	requires(sl::traits::is_constructible_from_v<VkSamplerCreateInfo, Args&&...>) {
		RenderProcessT const& proc = static_cast<RenderProcessT const&>(*this);
		if(!this->last_clear_frame.matches(proc.frame_count()))
			return errc::buffer_needs_changes_applied; //TODO


		const sl::index_t alloc_idx = this->allocation_index();

		_sampler_infos[alloc_idx].emplace_back(sl::forward<Args>(args)...);


		result<image_sampler> sampler_result = acma::make<image_sampler>(
			proc.vulkan_functions_ptr(),
			proc.logical_device_ptr(),
			_sampler_infos[alloc_idx].back()
		);
		if(!sampler_result.has_value()) [[unlikely]] {
			_sampler_infos[alloc_idx].pop_back();
			return sampler_result.error();
		}
		_samplers[alloc_idx].push_back(*sl::move(sampler_result));

		const sl::size_t total_descriptor_count = this->size();
		const std::unique_ptr<VkDescriptorImageInfo[]> descriptor_infos = std::make_unique_for_overwrite<VkDescriptorImageInfo[]>(total_descriptor_count);
		for(sl::index_t i = 0; i < total_descriptor_count; ++i) {
			new (&descriptor_infos[i]) VkDescriptorImageInfo{
				.sampler{_samplers[alloc_idx][i]}
			};
		}
		return this->update_descriptors({descriptor_infos.get(), total_descriptor_count});
	}
}

namespace acma::vk {
	template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
	requires(AssetHeapConfigs[K].usage == asset_usage_policy::sampler)
	constexpr void   asset_heap<K, AssetHeapConfigs, RenderProcessT>::
	clear() noexcept {
		_samplers[this->allocation_index()].clear();
		_sampler_infos[this->allocation_index()].clear();

		RenderProcessT const& proc = static_cast<RenderProcessT const&>(*this);
		this->last_clear_frame = proc.frame_count();
	}
}
