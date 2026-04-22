#pragma once
#include "sirius/vulkan/memory/asset_heap.fwd.hpp"

#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/asset_heap_key_t.hpp"
#include "sirius/core/asset_heap_config.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/display/image_sampler.hpp"
#include "sirius/vulkan/memory/buffer.hpp"
#include "sirius/vulkan/memory/descriptor_set_layout.hpp"
#include "sirius/vulkan/memory/descriptor_pool.hpp"
#include "sirius/vulkan/memory/descriptor_set.hpp"
#include "sirius/vulkan/memory/image.hpp"
#include "sirius/vulkan/memory/allocation_counts.hpp"
#include "sirius/vulkan/memory/texture_data_info.hpp"
#include "sirius/core/memory_management.fwd.hpp"


namespace acma::vk {
	template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
	class asset_heap {
	public:
		template<typename T>
		friend struct ::acma::impl::make;
	public:
		result<void> initialize() noexcept;
	public:
		constexpr static asset_heap_config config = AssetHeapConfigs[K];
		constexpr static sl::size_t allocation_count = impl::allocation_counts[config.coupling];

	public:
		constexpr sl::size_t total_size() const noexcept { return _images.size() + _sampler_infos.size(); }
		//constexpr sl::size_t capacity() const noexcept { return allocated_bytes; }

	public:
		constexpr auto const& descriptor_sets() const& noexcept { return _descriptor_sets[this->allocation_index()]; }
		constexpr auto const& descriptor_set_handles() const& noexcept { return _descriptor_set_handles[this->allocation_index()]; }

	public:
		template<typename T>
		constexpr result<void> push_back(T&& t) 
		noexcept(sl::traits::is_noexcept_constructible_from_v<VkSamplerCreateInfo, T&&>)
		requires(sl::traits::is_constructible_from_v<VkSamplerCreateInfo, T&&>);

		template<typename... Args>
		constexpr result<void> emplace_back(Args&&... args)
		noexcept(sl::traits::is_noexcept_constructible_from_v<VkSamplerCreateInfo, Args&&...>)
		requires(sl::traits::is_constructible_from_v<VkSamplerCreateInfo, Args&&...>);
		
	public:
	 	// template<sl::index_t J, sl::size_t N, auto BufferConfigs>
		// constexpr result<void> emplace_back(buffer<BufferKey, BufferConfigs, RenderProcessT> const& uniform_buffer) noexcept
		// requires((buffer<BufferKey, BufferConfigs, RenderProcessT>::config.usage & buffer_usage_policy::uniform) == buffer_usage_policy::uniform);

	 	// template<sl::index_t J, sl::size_t N, auto BufferConfigs>
		// constexpr result<void> try_emplace_back(buffer<BufferKey, BufferConfigs, RenderProcessT> const& uniform_buffer) noexcept
		// requires((buffer<BufferKey, BufferConfigs, RenderProcessT>::config.usage & buffer_usage_policy::uniform) == buffer_usage_policy::uniform);

	public:
	 	template<buffer_key_t BufferKey, auto BufferConfigs>
		constexpr result<void> emplace_back(buffer<BufferKey, BufferConfigs, RenderProcessT> const& texture_data_buffer) noexcept
		requires((buffer<BufferKey, BufferConfigs, RenderProcessT>::config.usage & buffer_usage_policy::texture_data) == buffer_usage_policy::texture_data);

	private:

		constexpr result<void> resize(sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> asset_counts) noexcept;


	private:
		constexpr result<void> make_images(std::vector<texture_data_info> const& texture_data_infos) noexcept;

		constexpr result<void> update_descriptors(
			sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> counts,
			sl::index_t alloc_idx
		) noexcept;

	private:
		result<void> make_pools(
			sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t> asset_counts,
			sl::index_t alloc_idx
		) noexcept;

	 	template<buffer_key_t BufferKey, auto BufferConfigs>
		result<void> upload_image_data(
			buffer<BufferKey, BufferConfigs, RenderProcessT> const& texture_data_buffer,
			sl::index_t alloc_idx,
			sl::index_t image_start_idx,
			sl::uint64_t timeout = std::numeric_limits<sl::uint64_t>::max()
		) noexcept;


	private:
		constexpr sl::index_t allocation_index() const& noexcept {
			return (static_cast<RenderProcessT const&>(*this).frame_count()) % allocation_count;
		}


	public:
		template<asset_heap_key_t DstK, buffer_key_t SrcK, auto _AssetHeapConfigs, auto _BufferConfigs, typename _RenderProcessT>
		friend constexpr result<void> acma::gpu_copy(
			vk::asset_heap<DstK, _AssetHeapConfigs, _RenderProcessT> & dst,
			vk::buffer<SrcK, _BufferConfigs, _RenderProcessT> const& src,
			sl::uint64_t timeout
		) noexcept;
		

	private:
		friend struct command_buffer;
	private:
		sl::array<allocation_count, sl::size_t> initialized_image_counts;
		sl::array<allocation_count, std::vector<image>> _images;
		sl::array<allocation_count, std::vector<asset_usage_policy_t>> _image_usages;
		sl::array<allocation_count, std::vector<image_sampler>> _samplers;
		sl::array<allocation_count, std::vector<VkSamplerCreateInfo>> _sampler_infos;
		
		sl::array<allocation_count, descriptor_pool> _descriptor_pools;
		sl::array<allocation_count, sl::array<asset_usage_policy::num_usage_policies, sl::uint32_t>> _descriptor_counts;
		sl::array<allocation_count, sl::array<asset_usage_policy::num_usage_policies, descriptor_set>> _descriptor_sets;
		sl::array<allocation_count, sl::array<asset_usage_policy::num_usage_policies, VkDescriptorSet>> _descriptor_set_handles;

		sl::array<asset_usage_policy::num_usage_policies, descriptor_set_layout> _descriptor_set_layouts;
		//sl::array<asset_usage_policy::num_usage_policies, VkWriteDescriptorSet> _descriptor_writes;
	};
}

#include "sirius/vulkan/memory/asset_heap.inl"