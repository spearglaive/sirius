#pragma once
#include "sirius/vulkan/memory/asset_heap.hpp"

#include <streamline/algorithm/aligned_to.hpp>
#include <streamline/functional/functor/forward_construct.hpp>

#include "sirius/vulkan/sync/semaphore.hpp"
#include "sirius/timeline/dedicated_command_group.hpp"
#include "sirius/core/memory_management.hpp"


namespace acma::vk {
	template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
	result<void>
		asset_heap<K, AssetHeapConfigs, RenderProcessT>::
	initialize() noexcept {
		RenderProcessT const& proc = static_cast<RenderProcessT const&>(*this);

		constexpr static sl::uint32_t stage_count = std::popcount(config.stages);
		for(asset_usage_policy_t j = 0; j < asset_usage_policy::num_usage_policies; ++j) {
			const VkDescriptorSetLayoutBinding set_layout_binding{
				.binding = 0,
				.descriptorType = vk::descriptor_types[j],
				.descriptorCount = std::min(std::min(
					proc.physical_device_ptr()->descriptor_count_limits[j] / stage_count,
					proc.physical_device_ptr()->per_stage_descriptor_count_limits[j]),
					proc.physical_device_ptr()->limits.maxPerStageResources / (stage_count * asset_usage_policy::num_usage_policies)
				),
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

			RESULT_TRY_MOVE(_descriptor_set_layouts[j], acma::make<descriptor_set_layout>(
				proc.vulkan_functions_ptr(),
				proc.logical_device_ptr(),
				set_layout_info
			));
		}

		for(sl::index_t i = 0; i < allocation_count; ++i) {
			RESULT_VERIFY(make_pools({}, i));
		}

		return {};
	}
}


namespace acma::vk {
	template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
	template<typename T>
	constexpr result<void>    asset_heap<K, AssetHeapConfigs, RenderProcessT>::
	push_back(T&& t) 
	noexcept(sl::traits::is_noexcept_constructible_from_v<VkSamplerCreateInfo, T&&>)
	requires(sl::traits::is_constructible_from_v<VkSamplerCreateInfo, T&&>) {
		const sl::index_t alloc_idx = allocation_index();

		_sampler_infos[alloc_idx].push_back(sl::forward<T>(t));

		RenderProcessT const& proc = static_cast<RenderProcessT const&>(*this);


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

		constexpr static asset_usage_policy_t usage = asset_usage_policy::sampler;


		sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> desired_descriptor_counts = _descriptor_counts[alloc_idx];
		++desired_descriptor_counts[usage];

		return update_descriptors(desired_descriptor_counts, alloc_idx);
	}

	template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
	template<typename... Args>
	constexpr result<void>    asset_heap<K, AssetHeapConfigs, RenderProcessT>::
	emplace_back(Args&&... args)
	noexcept(sl::traits::is_noexcept_constructible_from_v<VkSamplerCreateInfo, Args&&...>)
	requires(sl::traits::is_constructible_from_v<VkSamplerCreateInfo, Args&&...>) {
		const sl::index_t alloc_idx = allocation_index();

		_sampler_infos[alloc_idx].emplace_back(sl::forward<Args>(args)...);

		RenderProcessT const& proc = static_cast<RenderProcessT const&>(*this);


		result<image_sampler> sampler_result = make<image_sampler>(proc.logical_device_ptr(), _sampler_infos[alloc_idx].back());
		if(!sampler_result.has_value()) [[unlikely]] {
			_sampler_infos[alloc_idx].pop_back();
			return sampler_result.error();
		}
		_samplers[alloc_idx].push_back(*sl::move(sampler_result));

		constexpr static asset_usage_policy_t usage = asset_usage_policy::sampler;
		
		
		sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> desired_descriptor_counts = _descriptor_counts[alloc_idx];
		++desired_descriptor_counts[usage];
		
		return update_descriptors(desired_descriptor_counts, alloc_idx);
	}
}


namespace acma::vk {
	template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
	template<buffer_key_t BufferKey, auto BufferConfigs>
	constexpr result<void>   asset_heap<K, AssetHeapConfigs, RenderProcessT>::
 	emplace_back(buffer<BufferKey, BufferConfigs, RenderProcessT> const& texture_data_buffer) noexcept
	requires((buffer<BufferKey, BufferConfigs, RenderProcessT>::config.usage & buffer_usage_policy::texture_data) == buffer_usage_policy::texture_data) {
		if(texture_data_buffer.texture_data_infos.empty() || texture_data_buffer.size() == 0) 
			return {};


		RESULT_VERIFY(make_images(texture_data_buffer.texture_data_infos));
		
		RESULT_VERIFY(gpu_copy(*this, texture_data_buffer));

		const sl::index_t alloc_idx = allocation_index();
		
		sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> desired_descriptor_counts = _descriptor_counts[alloc_idx];
		for(sl::index_t i = 0; i < texture_data_buffer.texture_data_infos.size(); ++i) {
			const asset_usage_policy_t usage = asset_usage_policy::sampled_image + static_cast<asset_usage_policy_t>(texture_data_buffer.texture_data_infos[i].usage);
			++desired_descriptor_counts[usage];
			_image_usages[alloc_idx].push_back(usage);
		}
		
		result<void> update_result = update_descriptors(desired_descriptor_counts, alloc_idx);
		if(!update_result.has_value()) [[unlikely]] {
			_image_usages[alloc_idx].resize(_image_usages[alloc_idx].size() - texture_data_buffer.texture_data_infos.size());
			return update_result.error();
		}
		
		initialized_image_counts[alloc_idx] = _images[alloc_idx].size();
		return {};
	}
}


namespace acma::vk {
	template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
	constexpr result<void>
		asset_heap<K, AssetHeapConfigs, RenderProcessT>::
	make_images(std::vector<texture_data_info> const& texture_data_infos) noexcept {
		RenderProcessT const& proc = static_cast<RenderProcessT const&>(*this);
		const sl::index_t alloc_idx = allocation_index();

		const sl::size_t old_size = _images[alloc_idx].size();
		_images[alloc_idx].reserve(_images[alloc_idx].size() + texture_data_infos.size());
		for(sl::size_t i = 0; i < texture_data_infos.size(); ++i) {
			RESULT_VERIFY_UNSCOPED(make<image>(
				proc.vulkan_functions_ptr(),
				proc.logical_device_ptr(),
				proc.allocator_ptr(),
				static_cast<VkImageCreateInfo>(texture_data_infos[i])
			), img_result);
			_images[alloc_idx].push_back(*std::move(img_result));
		}

		initialized_image_counts[alloc_idx] = old_size;
		return {};
	}


	template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
	constexpr result<void>    asset_heap<K, AssetHeapConfigs, RenderProcessT>::
	update_descriptors(
		sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> counts,
		sl::index_t alloc_idx
	) noexcept {
		for(sl::index_t i = 0; i < asset_usage_policy::num_usage_policies; ++i) {
			if(counts[i] > _descriptor_counts[alloc_idx][i]) {
				RESULT_VERIFY(make_pools(counts, alloc_idx));
				break;
			}
		}
		_descriptor_counts[allocation_index()] = counts;

		sl::array<asset_usage_policy::num_usage_policies, std::vector<VkDescriptorImageInfo>> descriptor_infos{};
		for(sl::index_t i = 0; i < _images[alloc_idx].size(); ++i) {
			descriptor_infos[_image_usages[alloc_idx][i]].push_back(VkDescriptorImageInfo{
				.imageView{*_images[alloc_idx][i].view_ref()},
				.imageLayout{_images[alloc_idx][i].current_layout}
			});
		}
		for(sl::index_t i = 0; i < _samplers[alloc_idx].size(); ++i) {
			descriptor_infos[asset_usage_policy::sampler].push_back(VkDescriptorImageInfo{
				.sampler{_samplers[alloc_idx][i]}
			});
		}

		for(asset_usage_policy_t usage_idx = 0; usage_idx < asset_usage_policy::num_usage_policies; ++usage_idx) {
			const sl::size_t descriptor_count = descriptor_infos[usage_idx].size();
			if(descriptor_count == 0) continue;

			const VkWriteDescriptorSet write{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = _descriptor_sets[alloc_idx][usage_idx],
				.dstBinding = 0,
				.descriptorCount = static_cast<sl::uint32_t>(descriptor_count),
				.descriptorType = vk::descriptor_types[usage_idx],
				.pImageInfo = descriptor_infos[usage_idx].data(),
			};
			_descriptor_sets[alloc_idx][usage_idx].update({&write, 1});
		}

		return {};
	}
}


namespace acma::vk {
	template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
	result<void>   asset_heap<K, AssetHeapConfigs, RenderProcessT>::
 	make_pools(
		sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> asset_counts,
		sl::index_t alloc_idx
	) noexcept {
		RenderProcessT const& proc = static_cast<RenderProcessT const&>(*this);
		
		sl::array<asset_usage_policy::num_usage_policies, VkDescriptorPoolSize> pool_sizes{};
		for(sl::index_t i = 0; i < asset_usage_policy::num_usage_policies; ++i) {
			const sl::uint32_t asset_count = std::max(asset_counts[i], _descriptor_counts[alloc_idx][i]);
			pool_sizes[i] = {
				.type = vk::descriptor_types[i],
				.descriptorCount = std::max(asset_count, static_cast<sl::uint32_t>(1))
			};
		}
		const VkDescriptorPoolCreateInfo pool_create_info{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VkDescriptorPoolCreateFlags{},
			.maxSets = asset_usage_policy::num_usage_policies,
			.poolSizeCount = pool_sizes.size(),
			.pPoolSizes = pool_sizes.data(),
		};

		//const sl::unique_ptr<descriptor_pool> old_descriptor_pool_ptr = std::exchange(_descriptor_pool_ptrs[alloc_idx], sl::unique_ptr<descriptor_pool>{new descriptor_pool});
		RESULT_TRY_MOVE(_descriptor_pools[alloc_idx], acma::make<descriptor_pool>(
			proc.vulkan_functions_ptr(),
			proc.logical_device_ptr(),
			pool_create_info
		));
		


		for(sl::index_t i = 0; i < asset_usage_policy::num_usage_policies; ++i) {
			//Some graphics drivers are bugged and tweak out when you pass 0 as the asset count
			//So we make sure that there is always at least 1
			const sl::uint32_t asset_count = std::max(asset_counts[i], _descriptor_counts[alloc_idx][i]);
			const sl::uint32_t descriptor_count = std::max(asset_count, static_cast<sl::uint32_t>(1));

			const VkDescriptorSetVariableDescriptorCountAllocateInfo variable_count_alloc_info{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
				.pNext = nullptr,
				.descriptorSetCount = 1,
				.pDescriptorCounts = &descriptor_count
			};
			const VkDescriptorSetAllocateInfo set_alloc_info{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				.pNext = &variable_count_alloc_info,
				.descriptorPool = _descriptor_pools[alloc_idx],
				.descriptorSetCount = 1,
				.pSetLayouts = &_descriptor_set_layouts[i]
			};
			RESULT_TRY_MOVE(_descriptor_sets[alloc_idx][i], acma::make<descriptor_set>(
				proc.vulkan_functions_ptr(),
				proc.logical_device_ptr(),
				//_descriptor_pool_ptrs[alloc_idx],
				set_alloc_info
			));
		};
		return {};
	}
}