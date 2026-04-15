#pragma once
#include "sirius/vulkan/memory/buffer.fwd.hpp"
#include <streamline/functional/functor/generic_stateful.hpp>

#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/asset_heap_config.hpp"
#include "sirius/graphics/core/texture_view.hpp"
#include "sirius/vulkan/core/command_buffer.fwd.hpp"
#include "sirius/core/buffer_config_table.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/core/vulkan_ptr.hpp"
#include "sirius/core/buffer_config.hpp"
#include "sirius/core/memory_management.hpp"
#include "sirius/vulkan/memory/buffer_allocation.hpp"
#include "sirius/vulkan/memory/allocation_counts.hpp"
#include "sirius/vulkan/memory/texture_data_info.hpp"
#include "sirius/core/memory_management.fwd.hpp"
#include "sirius/vulkan/memory/asset_heap.fwd.hpp"
//#include "sirius/core/render_process.fwd.hpp"
//#include "sirius/core/frames_in_flight.def.hpp"
//#include "sirius/vulkan/memory/asset_group.hpp"
//#include "sirius/vulkan/memory/descriptor_heap.hpp"


//__D2D_DECLARE_VK_TRAITS_VMA(VkBuffer);




namespace acma::vk::impl {
	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
	struct buffer_properties {
		constexpr static buffer_config config = BufferConfigs[K];
		constexpr static sl::size_t allocation_count = allocation_counts[config.coupling];

	protected:
		constexpr sl::index_t allocation_index() const noexcept;
	public:
		friend command_buffer;
	};
}

namespace acma::vk::impl {
	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
	class buffer_base :
		public buffer_properties<K, BufferConfigs, RenderProcessT>
	{
		using base_type = buffer_properties<K, BufferConfigs, RenderProcessT>;
	public:
		using base_type::config;
		using base_type::allocation_count;
	public:
        result<void> initialize() noexcept;

	public:
		constexpr std::byte const* data() const noexcept requires(memory_policy::is_cpu_visible(config.memory))  { return std::launder(reinterpret_cast<std::byte const*>(buff_alloc_ptrs[this->allocation_index()]->allocation_info.pMappedData)); }
		constexpr std::byte      * data()       noexcept requires(memory_policy::is_cpu_writable(config.memory)) { return std::launder(reinterpret_cast<std::byte      *>(buff_alloc_ptrs[this->allocation_index()]->allocation_info.pMappedData)); }

		constexpr sl::size_t size() const noexcept { return data_bytes[this->allocation_index()]; }
		constexpr sl::size_t size_bytes() const noexcept { return data_bytes[this->allocation_index()]; }
		constexpr sl::size_t capacity() const noexcept { return buff_alloc_ptrs[this->allocation_index()]->creation_info.size; }
		constexpr sl::size_t capacity_bytes() const noexcept { return buff_alloc_ptrs[this->allocation_index()]->creation_info.size; }

		constexpr gpu_address_t gpu_address() const noexcept { return buff_alloc_ptrs[this->allocation_index()]->device_address; }
		constexpr VkBuffer handle() const noexcept { return buff_alloc_ptrs[this->allocation_index()]->handle; }
		constexpr operator VkBuffer() const noexcept { return handle(); }
		
		constexpr auto&& allocation_ptr(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.buff_alloc_ptrs[self.allocation_index()]); }
	private:
		constexpr auto&& allocated_bytes(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.buff_alloc_ptrs[self.allocation_index()]->allocation_info.size); }

	public:
		constexpr result<void> reserve(sl::size_t new_capacity_bytes) noexcept;

	public:
		constexpr void clear() noexcept;

		constexpr result<void> resize(sl::size_t count_bytes) noexcept;
		constexpr result<void> try_resize(sl::size_t count_bytes) noexcept;

	public:
		friend ::acma::vk::command_buffer;

	protected:
		sl::array<allocation_count, buffer_allocation_unique_ptr> buff_alloc_ptrs;
        sl::array<allocation_count, sl::size_t> data_bytes;
		sl::uoffset_t offset;
        VkBufferUsageFlags flags;
		VkDescriptorType descriptor_type;
	};


	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
	requires(
		buffer_properties<K, BufferConfigs, RenderProcessT>::config.memory == memory_policy::push_constant
	)
	class buffer_base<K, BufferConfigs, RenderProcessT> :
		public buffer_properties<K, BufferConfigs, RenderProcessT>
	{
		using base_type = buffer_properties<K, BufferConfigs, RenderProcessT>;
	public:
		using base_type::config;
		using base_type::allocation_count;
	public:
        result<void> initialize() noexcept { return {}; }

	public:
		constexpr std::byte const* data() const noexcept { return bytes[this->allocation_index()].data(); }
		constexpr std::byte      * data()       noexcept { return bytes[this->allocation_index()].data(); }

		consteval sl::size_t size() const noexcept { return config.initial_capacity_bytes; }
		consteval sl::size_t size_bytes() const noexcept { return config.initial_capacity_bytes; }
		consteval sl::size_t capacity() const noexcept { return config.initial_capacity_bytes; }
		consteval sl::size_t capacity_bytes() const noexcept { return config.initial_capacity_bytes; }
		
	public:
		friend ::acma::vk::command_buffer;

	protected:
		sl::array<allocation_count, sl::array<config.initial_capacity_bytes, std::byte>> bytes;
	};
}


namespace acma::vk {
	//Directly modifyable
	//Assumes that reads/writes are done safely
	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
	requires(
		!(impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data)) &&
		impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
	)
    class buffer<K, BufferConfigs, RenderProcessT> :
		public impl::buffer_base<K, BufferConfigs, RenderProcessT>
	{
	protected:
		using base_type = impl::buffer_base<K, BufferConfigs, RenderProcessT>;
	public:
		using base_type::config;
		using base_type::allocation_count;
	public:
		// static_assert(
		// 	!(buffer<K, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data)) &&
		// 	buffer<K, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
		// );
	public:
		template<buffer_key_t DstK>
		constexpr result<void> upload_to(
			buffer<DstK, BufferConfigs, RenderProcessT>& dst,
			sl::uoffset_t dst_offset = 0
		) noexcept;

		template<buffer_key_t DstK>
		constexpr result<void> try_upload_to(
			buffer<DstK, BufferConfigs, RenderProcessT>& dst,
			sl::uoffset_t dst_offset = 0
		) noexcept;

	public:
		template<typename T>
		constexpr result<void> push_back(T&& t) 
		noexcept(sl::traits::is_noexcept_constructible_from_v<T, T&&>)
		requires(sl::traits::is_constructible_from_v<T, T&&> && config.memory != memory_policy::push_constant);

		template<typename T>
		constexpr result<void> try_push_back(T&& t)
		noexcept(sl::traits::is_noexcept_constructible_from_v<T, T&&>)
		requires(sl::traits::is_constructible_from_v<T, T&&>);


		template<typename T, typename... Args>
		constexpr result<void> emplace_back(Args&&... args)
		noexcept(sl::traits::is_noexcept_constructible_from_v<T, Args&&...>)
		requires(sl::traits::is_constructible_from_v<T, Args&&...> && config.memory != memory_policy::push_constant);

		template<typename T, typename... Args>
		constexpr result<void> try_emplace_back(Args&&... args)
		noexcept(sl::traits::is_noexcept_constructible_from_v<T, Args&&...>)
		requires(sl::traits::is_constructible_from_v<T, Args&&...>);


	protected:
		template<typename T>
		constexpr result<void> push_to(sl::uoffset_t offset, T&& t) 
		noexcept(sl::traits::is_noexcept_constructible_from_v<T, T&&>);

		template<typename T, typename... Args>
		constexpr result<void> emplace_to(sl::uoffset_t dst_offset, Args&&... args)
		noexcept(sl::traits::is_noexcept_constructible_from_v<T, Args&&...>);
    };
}

namespace acma::vk {
	//Directly modifiable (specifically for textures)
	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
	requires (
		static_cast<bool>(impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data))
	)
	class buffer<K, BufferConfigs, RenderProcessT> : 
		public impl::buffer_base<K, BufferConfigs, RenderProcessT>
	{
	protected:
		using base_type = impl::buffer_base<K, BufferConfigs, RenderProcessT>;
	public:
		using base_type::config;
		using base_type::allocation_count;

	public:
		constexpr void clear() noexcept;
	public:
		constexpr result<void> push_back(texture_view t) noexcept;

		constexpr result<void> try_push_back(texture_view t) noexcept;

	private:	
		template<asset_heap_key_t J, auto AssetHeapConfigs, typename _RenderProcessT>
		friend class asset_heap;

		template<asset_heap_key_t DstK, buffer_key_t SrcK, auto AssetHeapConfigs, auto _BufferConfigs, typename _RenderProcessT>
		friend constexpr result<void> acma::gpu_copy(
			vk::asset_heap<DstK, AssetHeapConfigs, _RenderProcessT> & dst,
			vk::buffer<SrcK, _BufferConfigs, _RenderProcessT> const& src,
			sl::uint64_t timeout
		) noexcept;

	private:
		std::vector<texture_data_info> texture_data_infos;
	};
}

// namespace acma::vk {
// 	//Directly modifiable buffer for descriptor heaps
// 	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
// 	requires (
// 		buffer<K, BufferConfigs, RenderProcessT>::config.usage == buffer_usage_policy::asset_heap_table &&
// 		buffer<K, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
// 	)
// 	class buffer<K, BufferConfigs, RenderProcessT> : 
// 		public impl::buffer_base<K, BufferConfigs, RenderProcessT>
// 	{
// 		struct asset_group_info : descriptor_info {
// 			sl::size_t count;
// 		};
// 	private:
// 		template<sl::index_t J>
// 		using uniform_buffer_segment = buffer<DstK, BufferConfigs, RenderProcessT>;
// 	protected:
// 		using base_type = impl::buffer_base<K, BufferConfigs, RenderProcessT>;
// 	public:
// 		using base_type::config;
// 		using base_type::allocation_count;
// 	public:
// 		template<asset_heap_config Config, sl::index_t... Js>
// 		constexpr result<void> emplace(
// 			asset_heap<Config, RenderProcessT> const& asset_heap,
// 			uniform_buffer_segment<Js> const&... uniform_buffs
// 		) noexcept
// 		requires((uniform_buffer_segment<Js>::config.usage == buffer_usage_policy::uniform) && ...);

// 		template<asset_heap_config Config, sl::index_t... Js>
// 		constexpr result<void> try_emplace(
// 			asset_heap<Config, RenderProcessT> const& asset_heap,
// 			uniform_buffer_segment<Js> const&... uniform_buffs
// 		) noexcept
// 		requires((uniform_buffer_segment<Js>::config.usage == buffer_usage_policy::uniform) && ...);


// 		template<asset_heap_config Config, sl::index_t... Js>
// 		constexpr result<void> emplace(
// 			uniform_buffer_segment<Js> const&... uniform_buffs
// 		) noexcept
// 		requires((uniform_buffer_segment<Js>::config.usage == buffer_usage_policy::uniform) && ...);

// 		template<asset_heap_config Config, sl::index_t... Js>
// 		constexpr result<void> try_emplace(
// 			uniform_buffer_segment<Js> const&... uniform_buffs
// 		) noexcept
// 		requires((uniform_buffer_segment<Js>::config.usage == buffer_usage_policy::uniform) && ...);
// 	private:
// 		template<bool ForceResize, asset_heap_config Config, sl::index_t... Js>
// 		constexpr result<void> write_descriptors(
// 			sl::bool_constant_type<ForceResize>,
// 			asset_heap<Config, RenderProcessT> const& asset_heap,
// 			uniform_buffer_segment<Js> const&... uniform_buffs
// 		) noexcept;


// 	public:
// 		friend ::acma::vk::command_buffer;

// 	private:
// 		sl::array<descriptor_heap::num_descriptor_heaps, sl::size_t> heap_offsets;
// 	};
// }

namespace acma::vk {
	//Not directly modifyable
	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
	requires (
		impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.memory == memory_policy::gpu_local
	)
	class buffer<K, BufferConfigs, RenderProcessT> : 
		public impl::buffer_base<K, BufferConfigs, RenderProcessT> {};
}


#include "sirius/vulkan/memory/buffer.inl"
