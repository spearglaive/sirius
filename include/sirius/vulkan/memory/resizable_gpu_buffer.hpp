#pragma once
#include "sirius/vulkan/memory/resizable_gpu_buffer.fwd.hpp"

#include <streamline/functional/functor/generic_stateful.hpp>
#include <streamline/functional/functor/invoke_each_result.hpp>

#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/asset_heap_config.hpp"
#include "sirius/graphics/core/texture_view.hpp"
#include "sirius/vulkan/core/command_buffer.fwd.hpp"
#include "sirius/core/buffer_config_table.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/core/buffer_config.hpp"
#include "sirius/core/memory_management.fwd.hpp"
#include "sirius/vulkan/memory/buffer_allocation.hpp"
#include "sirius/vulkan/memory/allocation_counts.hpp"
#include "sirius/vulkan/memory/texture_data_info.hpp"
#include "sirius/core/memory_management.fwd.hpp"
#include "sirius/vulkan/memory/asset_heap.fwd.hpp"
#include "sirius/vulkan/memory/clear_frame.hpp"
#include "sirius/core/render_process.fwd.hpp"
#include "sirius/core/render_process_core.fwd.hpp"



namespace acma::vk::impl {
	template<buffer_config BufferConfig, sl::size_t CommandGroupCount>
	struct resizable_gpu_buffer_properties {
		constexpr static buffer_config config = BufferConfig;
		constexpr static sl::size_t allocation_count = allocation_counts[BufferConfig.access];
		constexpr static bool requires_clear_every_frame = impl::requires_clear<BufferConfig.access, BufferConfig.memory>;

	protected:
		constexpr sl::index_t allocation_index() const& noexcept;
	public:
		constexpr render_process_core<CommandGroupCount> const& render_proc_core() const noexcept { return *proc_ptr; }
		constexpr render_process_core<CommandGroupCount>      & render_proc_core()       noexcept { return *proc_ptr; }

	public:
		constexpr resizable_gpu_buffer_properties(render_process_core<CommandGroupCount>& proc_core) noexcept : proc_ptr(std::addressof(proc_core)) {}
	protected:
		constexpr resizable_gpu_buffer_properties() noexcept = default;
		template<typename> friend struct ::acma::impl::make;
		template<typename, auto> friend struct ::sl::functor::invoke_each_result;

	private:
		sl::reference_ptr<render_process_core<CommandGroupCount>> proc_ptr;
	public:
		friend command_buffer;
	};
}

namespace acma::vk::impl {
	template<buffer_config BufferConfig, sl::size_t CommandGroupCount>
	class resizable_gpu_buffer_base :
		public resizable_gpu_buffer_properties<BufferConfig, CommandGroupCount>
	{
		using base_type = resizable_gpu_buffer_properties<BufferConfig, CommandGroupCount>;
	public:
		using base_type::allocation_count;
		using base_type::base_type;
	public:
        result<void> initialize() noexcept;

	public:
		constexpr sl::reference_ptr<std::byte const> data(this auto const& self) noexcept requires(memory_policy::is_cpu_visible(BufferConfig.memory))  { return {std::launder(reinterpret_cast<std::byte const*>(self.buff_alloc_ptrs[self.allocation_index()]->allocation_info.pMappedData))}; }
		constexpr sl::reference_ptr<std::byte      > data(this auto      & self) noexcept requires(memory_policy::is_cpu_writable(BufferConfig.memory)) { return {std::launder(reinterpret_cast<std::byte      *>(self.buff_alloc_ptrs[self.allocation_index()]->allocation_info.pMappedData))}; }
		constexpr sl::reference_ptr<std::byte      > data(this auto      &&) noexcept = delete;

		constexpr sl::size_t size          (this auto const& self) noexcept { return self.data_bytes[self.allocation_index()]; }
		constexpr sl::size_t size_bytes    (this auto const& self) noexcept { return self.data_bytes[self.allocation_index()]; }
		constexpr sl::size_t capacity      (this auto const& self) noexcept { return self.buff_alloc_ptrs[self.allocation_index()]->creation_info.size; }
		constexpr sl::size_t capacity_bytes(this auto const& self) noexcept { return self.buff_alloc_ptrs[self.allocation_index()]->creation_info.size; }
		constexpr gpu_address_t gpu_address(this auto const& self) noexcept { return self.buff_alloc_ptrs[self.allocation_index()]->device_address; }

		constexpr VkBuffer handle  (this auto const& self) noexcept { return self.buff_alloc_ptrs[self.allocation_index()]->handle; }
		constexpr operator VkBuffer(this auto const& self) noexcept { return self.handle(); }

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
	protected:
		[[no_unique_address]] impl::clear_frame<BufferConfig.access, BufferConfig.memory> last_clear_frame;
	};


	template<buffer_config BufferConfig, sl::size_t CommandGroupCount>
	requires(BufferConfig.memory == memory_policy::push_constant)
	class resizable_gpu_buffer_base<BufferConfig, CommandGroupCount> :
		public resizable_gpu_buffer_properties<BufferConfig, CommandGroupCount>
	{
		using base_type = resizable_gpu_buffer_properties<BufferConfig, CommandGroupCount>;
	public:
		using base_type::allocation_count;
		using base_type::base_type;
	public:
        result<void> initialize() noexcept { bytes = {}; return {}; }

	public:
		constexpr sl::reference_ptr<std::byte const> data(this auto const& self) noexcept { return {self.bytes[self.allocation_index()].data()}; }
		constexpr sl::reference_ptr<std::byte      > data(this auto      & self) noexcept { return {self.bytes[self.allocation_index()].data()}; }
		constexpr sl::reference_ptr<std::byte      > data(this auto      &&) noexcept = delete;

		consteval static sl::size_t size()           noexcept { return BufferConfig.initial_capacity_bytes; }
		consteval static sl::size_t size_bytes()     noexcept { return BufferConfig.initial_capacity_bytes; }
		consteval static sl::size_t capacity()       noexcept { return BufferConfig.initial_capacity_bytes; }
		consteval static sl::size_t capacity_bytes() noexcept { return BufferConfig.initial_capacity_bytes; }

	public:
		friend ::acma::vk::command_buffer;

	protected:
		sl::array<allocation_count, sl::array<BufferConfig.initial_capacity_bytes, std::byte>> bytes;
	};
}


namespace acma::vk::generic {
	//Directly modifyable
	//Assumes that reads/writes are done safely
	template<buffer_config BufferConfig, sl::size_t CommandGroupCount>
	requires(
		!(BufferConfig.usage & (buffer_usage_policy::texture_data)) &&
		BufferConfig.memory != memory_policy::gpu_local
	)
    class resizable_gpu_buffer<sl::constant_type<buffer_config, BufferConfig>, sl::size_constant_type<CommandGroupCount>> :
		public impl::resizable_gpu_buffer_base<BufferConfig, CommandGroupCount>
	{
	protected:
		using base_type = impl::resizable_gpu_buffer_base<BufferConfig, CommandGroupCount>;
	public:
		using base_type::allocation_count;
		using base_type::base_type;
	public:
		// static_assert(
		// 	!(resizable_gpu_buffer<BufferConfig>::BufferConfig.usage & (buffer_usage_policy::texture_data)) &&
		// 	resizable_gpu_buffer<BufferConfig>::BufferConfig.memory != memory_policy::gpu_local
		// );
	public:
		template<sl::traits::specialization_of<generic::resizable_gpu_buffer> DstBufferT>
		constexpr result<void> upload_to(
			DstBufferT& dst,
			sl::uoffset_t dst_offset = 0
		) noexcept;

		template<sl::traits::specialization_of<generic::resizable_gpu_buffer> DstBufferT>
		constexpr result<void> try_upload_to(
			DstBufferT& dst,
			sl::uoffset_t dst_offset = 0
		) noexcept;

	public:
		template<typename T, typename U>
		constexpr result<T*> push_back(U&& u)
		noexcept(sl::traits::is_noexcept_constructible_from_v<T, U&&>)
		requires(sl::traits::is_constructible_from_v<T, U&&> && BufferConfig.memory != memory_policy::push_constant);

		template<typename T, typename U>
		constexpr result<T*> try_push_back(U&& u)
		noexcept(sl::traits::is_noexcept_constructible_from_v<T, U&&>)
		requires(sl::traits::is_constructible_from_v<T, U&&>);


		template<typename T, typename... Args>
		constexpr result<T*> emplace_back(Args&&... args)
		noexcept(sl::traits::is_noexcept_constructible_from_v<T, Args&&...>)
		requires(sl::traits::is_constructible_from_v<T, Args&&...> && BufferConfig.memory != memory_policy::push_constant);

		template<typename T, typename... Args>
		constexpr result<T*> try_emplace_back(Args&&... args)
		noexcept(sl::traits::is_noexcept_constructible_from_v<T, Args&&...>)
		requires(sl::traits::is_constructible_from_v<T, Args&&...>);


	protected:
		template<typename T, typename U>
		constexpr result<T*> push_to(sl::uoffset_t offset, U&& u)
		noexcept(sl::traits::is_noexcept_constructible_from_v<T, U&&>);

		template<typename T, typename... Args>
		constexpr result<T*> emplace_to(sl::uoffset_t dst_offset, Args&&... args)
		noexcept(sl::traits::is_noexcept_constructible_from_v<T, Args&&...>);
    };
}

namespace acma::vk::generic {
	//Directly modifiable (specifically for textures)
	template<buffer_config BufferConfig, sl::size_t CommandGroupCount>
	requires (
		static_cast<bool>(BufferConfig.usage & (buffer_usage_policy::texture_data))
	)
	class resizable_gpu_buffer<sl::constant_type<buffer_config, BufferConfig>, sl::size_constant_type<CommandGroupCount>> :
		public impl::resizable_gpu_buffer_base<BufferConfig, CommandGroupCount>
	{
	protected:
		using base_type = impl::resizable_gpu_buffer_base<BufferConfig, CommandGroupCount>;
	public:
		using base_type::allocation_count;
		using base_type::base_type;
	public:
        result<void> initialize() noexcept;

	public:
		constexpr void clear() noexcept;
	public:
		constexpr result<void> push_back(texture_view t) noexcept;

		constexpr result<void> try_push_back(texture_view t) noexcept;

	private:
		template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
		friend class acma::vk::asset_heap;

		template<sl::size_t _CommandGroupCount, sl::traits::specialization_of<vk::generic::resizable_gpu_buffer> SrcBufferT>
		friend constexpr result<void> acma::gpu_copy(
			render_process_core<_CommandGroupCount> const& process_core,
			std::span<vk::image> dst,
			SrcBufferT const& src,
			sl::uint64_t timeout
		) noexcept;

	private:
		std::vector<texture_data_info> texture_data_infos;
	};
}


namespace acma::vk::generic {
	//Not directly modifyable
	template<buffer_config BufferConfig, sl::size_t CommandGroupCount>
	requires (
		BufferConfig.memory == memory_policy::gpu_local
	)
	class resizable_gpu_buffer<sl::constant_type<buffer_config, BufferConfig>, sl::size_constant_type<CommandGroupCount>> :
		public impl::resizable_gpu_buffer_base<BufferConfig, CommandGroupCount> {
	protected:
		using base_type = impl::resizable_gpu_buffer_base<BufferConfig, CommandGroupCount>;
	public:
		using base_type::allocation_count;
		using base_type::base_type;
	};
}

#include "sirius/vulkan/memory/resizable_gpu_buffer.inl"
