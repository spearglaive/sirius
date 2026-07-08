#pragma once
#include "sirius/core/render_process_core.fwd.hpp"

#include <memory>
#include <span>
#include <streamline/memory/reference_ptr.hpp>
#include <streamline/functional/functor/generic_stateless.hpp>

#include "sirius/core/frames_in_flight.def.hpp"
#include "sirius/vulkan/memory/allocator.hpp"
#include "sirius/vulkan/core/command_buffer.hpp"
#include "sirius/vulkan/core/command_pool.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/device/physical_device.hpp"
#include "sirius/core/memory_management.fwd.hpp"


namespace acma {
	//This class exists because these types need to be the first base class of render process
	//This is so that it gets destroyed after all the buffers and asset heaps
	//Theyre all smart pointers because these specific types also need to be stored
	template<sl::size_t CommandGroupCount>
	class render_process_core {
	public:
		constexpr static sl::size_t frames_in_flight = D2D_FRAMES_IN_FLIGHT;
		constexpr static sl::size_t command_buffer_count = CommandGroupCount;

	public:
		constexpr render_process_core() noexcept = default;
		constexpr ~render_process_core() noexcept = default;
	public:
		render_process_core(render_process_core const&) = delete;
		render_process_core(render_process_core&&) = delete;
		render_process_core& operator=(render_process_core const&) = delete;
		render_process_core& operator=(render_process_core&&) = delete;

	public:
		//TODO?: rename these to *_ref_ptr instead of *_ptr?
		constexpr sl::reference_ptr<const vk::function_table > vulkan_functions_ptr() const& noexcept { return {std::addressof(_vulkan_functions)}; }
		constexpr sl::reference_ptr<const vk::physical_device> physical_device_ptr()  const& noexcept { return _physical_device_ptr; }
		constexpr sl::reference_ptr<const vk::logical_device > logical_device_ptr()   const& noexcept { return {std::addressof(_logical_device)}; }
		constexpr sl::reference_ptr<const vk::allocator      > allocator_ptr()        const& noexcept { return {std::addressof(_allocator)}; }
	public:
		constexpr std::span<const vk::command_pool                                      , impl::command_pool_count> command_pools            () const& noexcept { return std::span<const vk::command_pool                                      , impl::command_pool_count>{_command_pools.data(), impl::command_pool_count}; }
		constexpr std::span<const sl::array<command_buffer_count, vk::command_buffer>   , frames_in_flight        > command_buffers          () const& noexcept { return std::span<const sl::array<command_buffer_count, vk::command_buffer>   , frames_in_flight        >{_command_buffers.data(), frames_in_flight}; }
		constexpr std::span<const sl::array<command_family::num_families, vk::semaphore>, frames_in_flight        > command_family_semaphores() const& noexcept { return std::span<const sl::array<command_family::num_families, vk::semaphore>, frames_in_flight        >{_command_family_semaphores.data(), frames_in_flight}; }
		constexpr std::span<const sl::array<command_buffer_count, vk::semaphore>        , frames_in_flight        > command_buffer_semaphores() const& noexcept { return std::span<const sl::array<command_buffer_count, vk::semaphore>        , frames_in_flight        >{_command_buffer_semaphores.data(), frames_in_flight}; }

		constexpr auto&& command_family_semaphore_values(this auto&& self) noexcept { return sl::forward<decltype(self)>(self)._command_family_semaphore_values; }
		constexpr auto&& command_buffer_semaphore_values(this auto&& self) noexcept { return sl::forward<decltype(self)>(self)._command_buffer_semaphore_values; }
	public:
		constexpr sl::size_t  frame_count() const noexcept { return _frame_count; }
		constexpr sl::index_t frame_index() const noexcept { return frame_count() % frames_in_flight; }
		constexpr sl::index_t next_frame_index() const noexcept { return (frame_count() + 1) % frames_in_flight; }

	public:
		constexpr result<sl::uint64_t> begin_dedicated_copy(sl::index_t command_group_idx, sl::uint64_t timeout) const& noexcept;
		constexpr result<void> end_dedicated_copy(sl::uint64_t wait_value, sl::index_t command_group_idx, sl::uint64_t timeout) const& noexcept;
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
		template<sl::size_t _CommandGroupCount, buffer_config DstConfig, buffer_config SrcConfig>
		friend constexpr result<void> acma::gpu_copy(
			render_process_core<_CommandGroupCount>& process,
			vk::buffer_allocation_unique_ptr& dst,
			sl::constant_type<buffer_config, DstConfig>,
			vk::buffer_allocation_unique_ptr const& src,
			sl::constant_type<buffer_config, SrcConfig>,
			sl::size_t size,
			sl::uoffset_t dst_offset,
			sl::uoffset_t src_offset
		) noexcept;


	protected:
		vk::function_table _vulkan_functions;
		sl::reference_ptr<vk::physical_device> _physical_device_ptr;
		vk::logical_device _logical_device;
		vk::allocator _allocator;
	protected:
		sl::array<impl::command_pool_count, vk::command_pool> _command_pools;
		sl::array<frames_in_flight, sl::array<command_buffer_count, vk::command_buffer>> _command_buffers;
		sl::array<frames_in_flight, sl::array<command_buffer_count, vk::semaphore>> _command_buffer_semaphores;
		sl::array<frames_in_flight, sl::array<command_family::num_families, vk::semaphore>> _command_family_semaphores;

		mutable sl::array<frames_in_flight, sl::array<command_buffer_count, sl::uint64_t>> _command_buffer_semaphore_values;
		mutable sl::array<frames_in_flight, sl::array<command_family::num_families, sl::uint64_t>> _command_family_semaphore_values;
	protected:
		sl::size_t _frame_count;
	};
}


#include "sirius/core/render_process_core.inl"
