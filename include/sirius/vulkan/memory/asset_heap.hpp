#pragma once
#include "sirius/vulkan/memory/asset_heap.fwd.hpp"

#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/asset_heap_key_t.hpp"
#include "sirius/core/asset_heap_config.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/display/image_sampler.hpp"
#include "sirius/vulkan/memory/buffer.hpp"
#include "sirius/vulkan/memory/clear_frame.hpp"
#include "sirius/vulkan/memory/descriptor_set_layout.hpp"
#include "sirius/vulkan/memory/descriptor_pool.hpp"
#include "sirius/vulkan/memory/descriptor_set.hpp"
#include "sirius/vulkan/memory/image.hpp"
#include "sirius/vulkan/memory/allocation_counts.hpp"
#include "sirius/vulkan/memory/texture_data_info.hpp"
#include "sirius/core/memory_management.fwd.hpp"


namespace acma::vk::impl {
	template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
	class asset_heap_base {
	public:
		constexpr static asset_heap_config config = AssetHeapConfigs[K];
		constexpr static sl::size_t allocation_count = impl::allocation_counts[config.coupling];
		constexpr static VkDescriptorType descriptor_type = vk::descriptor_types[config.usage];
		constexpr static bool requires_clear_every_frame = impl::requires_clear<config.coupling, config.image_memory>;

	public:
		constexpr result<void> initialize() noexcept;

	public:
		constexpr descriptor_set        const& set()        const& noexcept { return _descriptor_sets[this->allocation_index()]; }
		constexpr descriptor_set_layout const& set_layout() const& noexcept { return _descriptor_set_layout; }
		//constexpr sl::array<allocation_count, > const& descriptor_set_handles() const& noexcept { return _descriptor_set_handles[this->allocation_index()]; }

	protected:
		constexpr sl::index_t allocation_index() const& noexcept {
			return (static_cast<RenderProcessT const&>(*this).frame_count()) % allocation_count;
		}


	protected:
		constexpr result<void> update_descriptors(
			std::span<const VkDescriptorImageInfo> infos
		) noexcept;

	protected:
		descriptor_set_layout _descriptor_set_layout;
		sl::array<allocation_count, descriptor_pool> _descriptor_pools;
		sl::array<allocation_count, sl::uint32_t> _descriptor_capacities;
		sl::array<allocation_count, descriptor_set> _descriptor_sets;
	protected:
		[[no_unique_address]] impl::clear_frame<config.coupling, config.image_memory> last_clear_frame;
	};
}


namespace acma::vk {
	template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
	class asset_heap : public impl::asset_heap_base<K, AssetHeapConfigs, RenderProcessT> {
		using base_type = impl::asset_heap_base<K, AssetHeapConfigs, RenderProcessT>;
	public:
		template<typename T>
		friend struct ::acma::impl::make;
	public:
		using base_type::config;
		using base_type::allocation_count;
		using base_type::descriptor_type;


	public:
	 	template<buffer_key_t BufferKey, auto BufferConfigs>
		constexpr result<void> emplace_back(buffer<BufferKey, BufferConfigs, RenderProcessT> const& texture_data_buffer) noexcept;

	public:
		constexpr image const* data() const& noexcept { return _images[this->allocation_index()].data(); }
		constexpr image      * data()      & noexcept { return _images[this->allocation_index()].data(); }
	public:
		constexpr sl::size_t size()       const noexcept { return _images[this->allocation_index()].size(); }
		constexpr sl::size_t size_bytes() const noexcept { return _images[this->allocation_index()].size() * sizeof(image); }

	public:
		constexpr void clear() noexcept;

	private:
		constexpr result<void> make_images(std::vector<texture_data_info> const& texture_data_infos) noexcept;


	private:
		sl::array<allocation_count, std::vector<image>> _images;
	};
}

namespace acma::vk {
	template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
	requires(AssetHeapConfigs[K].usage == asset_usage_policy::sampler)
	class asset_heap<K, AssetHeapConfigs, RenderProcessT> : public impl::asset_heap_base<K, AssetHeapConfigs, RenderProcessT> {
		using base_type = impl::asset_heap_base<K, AssetHeapConfigs, RenderProcessT>;
	public:
		template<typename T>
		friend struct ::acma::impl::make;
	public:
		using base_type::config;
		using base_type::allocation_count;
		using base_type::descriptor_type;

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
		constexpr image_sampler const* data() const& noexcept { return _samplers[this->allocation_index()].data(); }
		constexpr image_sampler      * data()      & noexcept { return _samplers[this->allocation_index()].data(); }
	public:
		constexpr sl::size_t size()       const noexcept { return _samplers[this->allocation_index()].size(); }
		constexpr sl::size_t size_bytes() const noexcept { return _samplers[this->allocation_index()].size() * sizeof(image_sampler); }

	public:
		constexpr void clear() noexcept;

	private:
		sl::array<allocation_count, std::vector<image_sampler>> _samplers;
		sl::array<allocation_count, std::vector<VkSamplerCreateInfo>> _sampler_infos;
	};
}

#include "sirius/vulkan/memory/asset_heap.inl"
