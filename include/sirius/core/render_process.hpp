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
#include "sirius/vulkan/memory/buffer_tuple.hpp"
#include "sirius/vulkan/memory/asset_heap_group.hpp"
#include "sirius/core/buffer_config_table.hpp"
#include "sirius/vulkan/sync/semaphore.hpp"
#include "sirius/core/asset_heap_key_t.hpp"



namespace acma {
	template<auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount, sl::size_t UserByteCount = 0>
	class render_process :
		public render_process_core<CommandGroupCount>,
		public vk::impl::asset_heap_group<
			sl::index_sequence_of_length_type<AssetHeapConfigs.size()>,
			AssetHeapConfigs,
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount, UserByteCount>
		>
	{

	public:
		constexpr static sl::size_t frames_in_flight = D2D_FRAMES_IN_FLIGHT;
		constexpr static sl::size_t command_buffer_count = CommandGroupCount;
	public:
		constexpr static buffer_config_table<BufferConfigs.size()> buffer_configs = BufferConfigs;
		constexpr static asset_heap_config_table<AssetHeapConfigs.size()> asset_heap_configs = AssetHeapConfigs;

	private:
		constexpr static sl::size_t N = BufferConfigs.size();
		constexpr static sl::size_t M = AssetHeapConfigs.size();

		constexpr static sl::lookup_table<N, buffer_key_t, sl::index_t> buffer_key_indices = sl::universal::make_deduced<sl::generic::lookup_table>(
			BufferConfigs, sl::functor::subscript<0>{}, sl::functor::identity_index{}
		);
		constexpr static sl::lookup_table<M, asset_heap_key_t, sl::index_t> asset_heap_key_indices = sl::universal::make_deduced<sl::generic::lookup_table>(
			AssetHeapConfigs, sl::functor::subscript<0>{}, sl::functor::identity_index{}
		);

		template<buffer_key_t K>
		using buffer_type = vk::resizable_gpu_buffer<buffer_configs[K], render_process>;
		template<asset_heap_key_t K>
		using asset_heap_type = vk::asset_heap<K, asset_heap_configs, render_process>;
	public:
		using callback_function_type = result<void>(render_process&, window&, timeline::state<UserByteCount>&) noexcept;
		using buffer_tuple_type = vk::impl::buffer_tuple_t<sl::index_sequence_of_length_type<N>, BufferConfigs, render_process>;


	public:
		template<buffer_key_t Key>
		constexpr auto&& operator[](this auto&& self, sl::constant_type<buffer_key_t, Key>) noexcept
		requires (BufferConfigs.contains(Key)) {
			return sl::forward_like<decltype(self)>(self._gpu_buffers[sl::index_constant<buffer_key_indices[Key]>]);
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
		inline std::span<const vk::semaphore>                   graphics_semaphores   () const& noexcept { return std::span<const vk::semaphore>                  {_graphics_semaphores.data(), _graphics_semaphores.size()}; }
		inline std::span<const vk::semaphore>                   pre_present_semaphores() const& noexcept { return std::span<const vk::semaphore>                  {_pre_present_semaphores.data(), _pre_present_semaphores.size()}; }
		inline std::span<const vk::semaphore, frames_in_flight> acquisition_semaphores() const& noexcept { return std::span<const vk::semaphore, frames_in_flight>{_acquisition_semaphores.data(), frames_in_flight}; }


		constexpr auto&& timeline_callbacks(this auto&& self) noexcept {return sl::forward_like<decltype(self)>(self._timeline_callbacks); }

		constexpr auto&& old_allocations(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._old_allocations); }


	protected:
		sl::array<timeline::callback_event::num_callback_events, std::vector<callback_function_type*>> _timeline_callbacks;

		sl::array<frames_in_flight, vk::semaphore> _acquisition_semaphores;
		std::vector<vk::semaphore> _graphics_semaphores;
		std::vector<vk::semaphore> _pre_present_semaphores;

		sl::array<frames_in_flight, std::vector<vk::buffer_allocation_unique_ptr>> _old_allocations;

	protected:
		buffer_tuple_type _gpu_buffers;
	};
}
