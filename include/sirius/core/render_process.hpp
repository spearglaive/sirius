#pragma once
#include "sirius/core/render_process.fwd.hpp"

#include <vector>
#include <memory>
#include <streamline/functional/functor/subscript.hpp>
#include <streamline/functional/functor/identity_index.hpp>
#include <streamline/functional/functor/generic_stateless.hpp>
#include <streamline/functional/functor/cast_static.hpp>
#include <streamline/memory/unique_ptr.hpp>

#include "sirius/core/render_process_core.hpp"
#include "sirius/core/window.fwd.hpp"
#include "sirius/core/frames_in_flight.def.hpp"
#include "sirius/timeline/callback_event.hpp"
#include "sirius/timeline/state.hpp"
#include "sirius/vulkan/memory/buffer_group.hpp"
#include "sirius/vulkan/memory/asset_heap_group.hpp"
#include "sirius/vulkan/memory/allocator.hpp"
#include "sirius/vulkan/core/command_buffer.hpp"
#include "sirius/vulkan/core/command_pool.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/device/physical_device.hpp"
#include "sirius/core/buffer_config_table.hpp"
#include "sirius/vulkan/sync/semaphore.hpp"
#include "sirius/core/asset_heap_key_t.hpp"
#include "sirius/core/memory_management.fwd.hpp"



namespace acma {
	template<auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
	class render_process :
		public impl::render_process_core,
		public vk::impl::buffer_group<
			sl::index_sequence_of_length_type<BufferConfigs.size()>,
			BufferConfigs,
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount>
		>,
		public vk::impl::asset_heap_group<
			sl::index_sequence_of_length_type<AssetHeapConfigs.size()>,
			AssetHeapConfigs,
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount>
		>
	{	

	public:
		constexpr static sl::size_t frames_in_flight = D2D_FRAMES_IN_FLIGHT;
		constexpr static sl::size_t command_buffer_count = CommandGroupCount;
	public:
		constexpr static buffer_config_table<BufferConfigs.size()> buffer_configs = BufferConfigs;
		constexpr static asset_heap_config_table<AssetHeapConfigs.size()> asset_heap_configs = AssetHeapConfigs;

	private:
		// constexpr static sl::size_t N = BufferConfigs.size();
		constexpr static sl::size_t M = AssetHeapConfigs.size();

		// constexpr static sl::lookup_table<N, buffer_key_t, sl::index_t> buffer_key_indices = sl::universal::make_deduced<sl::generic::lookup_table>(
		// 	BufferConfigs, sl::functor::subscript<0>{}, sl::functor::identity_index{}
		// );
		constexpr static sl::lookup_table<M, asset_heap_key_t, sl::index_t> asset_heap_key_indices = sl::universal::make_deduced<sl::generic::lookup_table>(
			AssetHeapConfigs, sl::functor::subscript<0>{}, []<sl::index_t I>(auto, sl::index_constant_type<I>){ return I; }
		);

		template<buffer_key_t K>
		using buffer_type = vk::buffer<K, buffer_configs, render_process>;
		template<asset_heap_key_t K>
		using asset_heap_type = vk::asset_heap<K, asset_heap_configs, render_process>;
	public:
		using callback_function_type = result<void>(render_process&, window&, timeline::state&) noexcept;


	public:
		template<buffer_key_t Key>
		constexpr auto&& operator[](this auto&& self, sl::constant_type<buffer_key_t, Key>) noexcept 
		requires (BufferConfigs.contains(Key)) {
			return static_cast<sl::copy_cvref_t<decltype(self), buffer_type<Key>>>(self);
		}
		
		
		template<buffer_key_t Key>
		constexpr auto&& get(this auto&& self, sl::constant_type<buffer_key_t, Key> = {}) noexcept 
		requires (BufferConfigs.contains(Key)) {
			return sl::forward_like<decltype(self)>(self[sl::constant<buffer_key_t, Key>]);
		}
		
	public:
		template<asset_heap_key_t Key>
		constexpr auto&& operator[](this auto&& self, sl::constant_type<asset_heap_key_t, Key>) noexcept 
		requires (AssetHeapConfigs.contains(Key)) {
			return static_cast<sl::copy_cvref_t<decltype(self), asset_heap_type<Key>>>(self);
		}
		
		template<asset_heap_key_t Key>
		constexpr auto&& get(this auto&& self, sl::constant_type<asset_heap_key_t, Key> = {}) noexcept 
		requires (AssetHeapConfigs.contains(Key)) {
			return sl::forward_like<decltype(self)>(self[sl::constant<asset_heap_key_t, Key>]);
		}

	public:
		//TODO?: rename these to *_ref_ptr instead of *_ptr?
		constexpr sl::reference_ptr<const vk::function_table                                   > vulkan_functions_ptr() const& noexcept { return _vulkan_functions_ptr; }
		constexpr sl::reference_ptr<const vk::physical_device                                  > physical_device_ptr()  const& noexcept { return _physical_device_ptr; }
		constexpr sl::reference_ptr<const vk::logical_device                                   > logical_device_ptr()   const& noexcept { return _logical_device_ptr; }
		constexpr sl::reference_ptr<const vk::allocator                                        > allocator_ptr()        const& noexcept { return _allocator_ptr; }
		constexpr sl::array<impl::command_pool_count, sl::reference_ptr<const vk::command_pool>> command_pool_ptrs()    const& noexcept { return sl::make<sl::array<impl::command_pool_count, sl::reference_ptr<const vk::command_pool>>>(_command_pool_ptrs); }
	public:
		inline std::span<const sl::array<command_buffer_count, vk::command_buffer>   , frames_in_flight> command_buffers          () const& noexcept { return std::span<const sl::array<command_buffer_count, vk::command_buffer>   , frames_in_flight>{_command_buffers.data(), frames_in_flight}; }
		inline std::span<const sl::array<command_family::num_families, vk::semaphore>, frames_in_flight> command_family_semaphores() const& noexcept { return std::span<const sl::array<command_family::num_families, vk::semaphore>, frames_in_flight>{_generic_timeline_sempahores.data(), frames_in_flight}; }
		inline std::span<const sl::array<command_buffer_count, vk::semaphore>        , frames_in_flight> command_buffer_semaphores() const& noexcept { return std::span<const sl::array<command_buffer_count, vk::semaphore>        , frames_in_flight>{_command_buffer_semaphores.data(), frames_in_flight}; }
		inline std::span<const vk::semaphore>                                                            graphics_semaphores      () const& noexcept { return std::span<const vk::semaphore>                                                           {_graphics_semaphores.data(), _graphics_semaphores.size()}; }
		inline std::span<const vk::semaphore>                                                            pre_present_semaphores   () const& noexcept { return std::span<const vk::semaphore>                                                           {_pre_present_semaphores.data(), _pre_present_semaphores.size()}; }
		inline std::span<const vk::semaphore, frames_in_flight>                                          acquisition_semaphores   () const& noexcept { return std::span<const vk::semaphore, frames_in_flight>                                         {_acquisition_semaphores.data(), frames_in_flight}; }


		constexpr auto&& command_family_semaphore_values(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._command_family_semaphore_values); }
		constexpr auto&& command_buffer_semaphore_values(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._command_buffer_semaphore_values); }

		constexpr auto&& timeline_callbacks(this auto&& self) noexcept {return sl::forward_like<decltype(self)>(self._timeline_callbacks); }

	public:	
		constexpr sl::size_t  frame_count() const noexcept { return _frame_count; }
		constexpr sl::index_t frame_index() const noexcept { return frame_count() % frames_in_flight; }
		constexpr sl::index_t next_frame_index() const noexcept { return (frame_count() + 1) % frames_in_flight; }

	public:
		constexpr bool has_dedicated_present_queue() const noexcept {
			return 
				_physical_device_ptr->queue_family_infos[command_family::graphics].index != 
				_physical_device_ptr->queue_family_infos[command_family::present].index;
		}
	private:
		//Any buffer to gpu_local buffer
		//(cpu_local_gpu_writable buffer to gpu_local buffer not allowed?)
		template<buffer_config DstConfig, buffer_config SrcConfig>
		constexpr result<void> buffer_copy(
			vk::buffer_allocation_unique_ptr& dst,
			sl::constant_type<buffer_config, DstConfig>,
			vk::buffer_allocation_unique_ptr const& src,
			sl::constant_type<buffer_config, SrcConfig>,
			sl::size_t size,
			sl::uoffset_t dst_offset = 0,
			sl::uoffset_t src_offset = 0
		) & noexcept
		requires(
			!memory_policy::is_cpu_visible(DstConfig.memory)
		);

		//Any host-visible buffer to any other host-visible buffer
		//(Any host-visible buffer to cpu_local_gpu_writable buffer not allowed)
		template<buffer_config DstConfig, buffer_config SrcConfig>
		constexpr result<void> buffer_copy(
			vk::buffer_allocation_unique_ptr& dst,
			sl::constant_type<buffer_config, DstConfig>,
			vk::buffer_allocation_unique_ptr const& src,
			sl::constant_type<buffer_config, SrcConfig>,
			sl::size_t size,
			sl::uoffset_t dst_offset = 0,
			sl::uoffset_t src_offset = 0
		) & noexcept
		requires(
			memory_policy::is_cpu_visible(DstConfig.memory) &&
			memory_policy::is_cpu_visible(SrcConfig.memory)
		);


		//gpu_local buffer to cpu_local_gpu_writable buffer
		//(gpu_local buffer to cpu_local_upload buffer not allowed)

		
	public:
		constexpr result<sl::uint64_t> begin_dedicated_copy(sl::index_t command_group_idx, sl::uint64_t timeout) & noexcept;
		constexpr result<void> end_dedicated_copy(sl::uint64_t wait_value, sl::index_t command_group_idx, sl::uint64_t timeout) const& noexcept;

	public:
		template<buffer_config DstConfig, buffer_config SrcConfig, typename RenderProcessT>
		friend constexpr result<void> acma::gpu_copy(
			RenderProcessT& process,
			vk::buffer_allocation_unique_ptr& dst,
			sl::constant_type<buffer_config, DstConfig>,
			vk::buffer_allocation_unique_ptr const& src,
			sl::constant_type<buffer_config, SrcConfig>,
			sl::size_t size,
			sl::uoffset_t dst_offset,
			sl::uoffset_t src_offset
		) noexcept;


	protected:
		//std::size_t frame_idx;
		sl::array<timeline::callback_event::num_callback_events, std::vector<callback_function_type*>> _timeline_callbacks;

		sl::array<frames_in_flight, sl::array<command_buffer_count, vk::command_buffer>> _command_buffers;
		sl::array<frames_in_flight, sl::array<command_buffer_count, vk::semaphore>> _command_buffer_semaphores;
		sl::array<frames_in_flight, sl::array<command_buffer_count, sl::uint64_t>> _command_buffer_semaphore_values;
		sl::array<frames_in_flight, sl::array<command_family::num_families, vk::semaphore>> _generic_timeline_sempahores;
		sl::array<frames_in_flight, sl::array<command_family::num_families, sl::uint64_t>> _command_family_semaphore_values;
		sl::array<frames_in_flight, vk::semaphore> _acquisition_semaphores;
		std::vector<vk::semaphore> _graphics_semaphores;
		std::vector<vk::semaphore> _pre_present_semaphores;

		sl::size_t _frame_count;
	};
}


#include "sirius/core/render_process.inl"
