#pragma once
#include "sirius/vulkan/memory/resizable_gpu_buffer.hpp"

#include <numeric>
#include <streamline/algorithm/aligned_to.hpp>
#include <streamline/functional/functor/address_of.hpp>
#include <streamline/functional/invoke.hpp>

#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/error.hpp"
#include "sirius/core/render_process.fwd.hpp"


namespace acma::vk::impl {
	template<buffer_config BufferConfig, sl::size_t CommandGroupCount>
    result<void>    resizable_gpu_buffer_base<BufferConfig, CommandGroupCount>::
	initialize() noexcept {
		for(sl::index_t i = 0; i < allocation_count; ++i) {
			RESULT_TRY_MOVE(this->buff_alloc_ptrs[i], gpu_allocate(this->render_proc_core(), sl::constant<buffer_config, BufferConfig>));
			this->data_bytes[i] = 0;
		}
		this->offset = 0;
		this->flags = VkBufferUsageFlags{};
		this->descriptor_type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
		this->last_clear_frame = 0;
        return {};
    }
}


namespace acma::vk::impl {
	template<buffer_config BufferConfig, sl::size_t CommandGroupCount>
    constexpr sl::index_t    resizable_gpu_buffer_properties<BufferConfig, CommandGroupCount>::
	allocation_index() const& noexcept {
		return (this->render_proc_core().frame_count()) % allocation_count;
	}
}



namespace acma::vk::impl {
	template<buffer_config BufferConfig, sl::size_t CommandGroupCount>
	constexpr result<void>    impl::resizable_gpu_buffer_base<BufferConfig, CommandGroupCount>::
	reserve(sl::size_t new_capacity_bytes) noexcept {
		//TODO: move all this to a gpu_realloc function
		if(new_capacity_bytes <= this->capacity_bytes())
			return {};

		const sl::index_t alloc_idx = this->allocation_index();
		if(new_capacity_bytes <= this->allocated_bytes()) {
			sl::invoke(this->render_proc_core().vulkan_functions_ptr()->vkDestroyBuffer, *this->render_proc_core().logical_device_ptr(), this->buff_alloc_ptrs[alloc_idx]->handle, nullptr);
			this->buff_alloc_ptrs[alloc_idx]->handle = VK_NULL_HANDLE;

			buffer_creation_info_t create_info = this->buff_alloc_ptrs[alloc_idx]->creation_info;
			create_info.size = new_capacity_bytes;

			__D2D_VULKAN_VERIFY(vmaCreateAliasingBuffer2(
				*this->render_proc_core().allocator_ptr(),
				this->buff_alloc_ptrs[alloc_idx]->allocation_handle,
				0,
				&create_info,
				&this->buff_alloc_ptrs[alloc_idx]->handle
			));
			this->buff_alloc_ptrs[alloc_idx]->creation_info = create_info;


			const VkBufferDeviceAddressInfo device_address_info{
				.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR,
				.buffer = this->buff_alloc_ptrs[alloc_idx]->handle
			};
			this->buff_alloc_ptrs[alloc_idx]->device_address = sl::invoke(this->render_proc_core().vulkan_functions_ptr()->vkGetBufferDeviceAddress, *this->render_proc_core().logical_device_ptr(), &device_address_info);
		}
		else {
			vk::buffer_allocation_unique_ptr old_alloc = std::move(this->buff_alloc_ptrs[alloc_idx]);
			RESULT_TRY_MOVE(this->buff_alloc_ptrs[alloc_idx], gpu_allocate_like(this->render_proc_core(), old_alloc, new_capacity_bytes));
			RESULT_VERIFY(gpu_copy(
				this->render_proc_core(),
				this->buff_alloc_ptrs[alloc_idx], sl::constant<buffer_config, BufferConfig>,
				old_alloc, sl::constant<buffer_config, BufferConfig>,
				old_alloc->creation_info.size
			));
		}
		return {};
	}
}


namespace acma::vk::impl {
	template<buffer_config BufferConfig, sl::size_t CommandGroupCount>
	constexpr void    impl::resizable_gpu_buffer_base<BufferConfig, CommandGroupCount>::
	clear() noexcept {
		this->data_bytes[this->allocation_index()] = 0;

		this->last_clear_frame = this->render_proc_core().frame_count();
	}


	template<buffer_config BufferConfig, sl::size_t CommandGroupCount>
	constexpr result<void>    impl::resizable_gpu_buffer_base<BufferConfig, CommandGroupCount>::
	resize(sl::size_t count_bytes) noexcept {
		RESULT_VERIFY(this->reserve(count_bytes));
		this->data_bytes[this->allocation_index()] = count_bytes;
		return {};
	}

	template<buffer_config BufferConfig, sl::size_t CommandGroupCount>
	constexpr result<void>    impl::resizable_gpu_buffer_base<BufferConfig, CommandGroupCount>::
	try_resize(sl::size_t count_bytes) noexcept {
		if(count_bytes > this->capacity_bytes())
			return errc::not_enough_memory;
		this->data_bytes[this->allocation_index()] = count_bytes;
		return {};
	}
}


namespace acma::vk::generic {
	template<buffer_config BufferConfig, sl::size_t CommandGroupCount>
	requires(BufferConfig.memory != memory_policy::gpu_local)
	template<sl::traits::specialization_of<generic::resizable_gpu_buffer> DstBufferT>
	constexpr result<void>    resizable_gpu_buffer<sl::constant_type<buffer_config, BufferConfig>, sl::size_constant_type<CommandGroupCount>>::
	upload_to(DstBufferT& dst, sl::uoffset_t dst_offset) noexcept {
		RESULT_VERIFY(dst.resize(dst_offset + this->size_bytes()));

		RESULT_VERIFY(gpu_copy(this->render_proc_core(), dst, *this, this->size_bytes(), dst_offset, 0));
		this->clear();
		return {};
	}

	template<buffer_config BufferConfig, sl::size_t CommandGroupCount>
	requires(BufferConfig.memory != memory_policy::gpu_local)
	template<sl::traits::specialization_of<generic::resizable_gpu_buffer> DstBufferT>
	constexpr result<void>    resizable_gpu_buffer<sl::constant_type<buffer_config, BufferConfig>, sl::size_constant_type<CommandGroupCount>>::
	try_upload_to(DstBufferT& dst, sl::uoffset_t dst_offset) noexcept {
		RESULT_VERIFY(dst.try_resize(dst_offset + this->size_bytes()));

		RESULT_VERIFY(gpu_copy(this->render_proc_core(), dst, *this, this->size_bytes(), dst_offset, 0));
		this->clear();
		return {};
	}
}

namespace acma::vk::generic {
	template<buffer_config BufferConfig, sl::size_t CommandGroupCount>
	requires(BufferConfig.memory != memory_policy::gpu_local)
	template<typename T, typename U>
	constexpr result<T*>    resizable_gpu_buffer<sl::constant_type<buffer_config, BufferConfig>, sl::size_constant_type<CommandGroupCount>>::
	push_back(U&& u)
	noexcept(sl::traits::is_noexcept_constructible_from_v<T, U&&>)
	requires(sl::traits::is_constructible_from_v<T, U&&> && BufferConfig.memory != memory_policy::push_constant) {
		const sl::size_t old_size = this->size_bytes();
		RESULT_VERIFY(this->resize(old_size + sizeof(T)));

		return this->template push_to<T>(old_size, sl::forward<U>(u));
	}

	template<buffer_config BufferConfig, sl::size_t CommandGroupCount>
	requires(BufferConfig.memory != memory_policy::gpu_local)
	template<typename T, typename U>
	constexpr result<T*>    resizable_gpu_buffer<sl::constant_type<buffer_config, BufferConfig>, sl::size_constant_type<CommandGroupCount>>::
	try_push_back(U&& u)
	noexcept(sl::traits::is_noexcept_constructible_from_v<T, U&&>)
	requires(sl::traits::is_constructible_from_v<T, U&&>) {
		const sl::size_t old_size = this->size_bytes();
		RESULT_VERIFY(this->try_resize(old_size + sizeof(T)));

		return this->template push_to<T>(old_size, sl::forward<U>(u));
	}
}

namespace acma::vk::generic {
	template<buffer_config BufferConfig, sl::size_t CommandGroupCount>
	requires(BufferConfig.memory != memory_policy::gpu_local)
	template<typename T, typename... Args>
	constexpr result<T*>    resizable_gpu_buffer<sl::constant_type<buffer_config, BufferConfig>, sl::size_constant_type<CommandGroupCount>>::
	emplace_back(Args&&... args)
	noexcept(sl::traits::is_noexcept_constructible_from_v<T, Args&&...>)
	requires(sl::traits::is_constructible_from_v<T, Args&&...> && BufferConfig.memory != memory_policy::push_constant) {
		const sl::size_t old_size = this->size_bytes();
		RESULT_VERIFY(this->resize(old_size + sizeof(T)));

		return this->template emplace_to<T>(old_size, sl::forward<Args>(args)...);
	}

	template<buffer_config BufferConfig, sl::size_t CommandGroupCount>
	requires(BufferConfig.memory != memory_policy::gpu_local)
	template<typename T, typename... Args>
	constexpr result<T*>    resizable_gpu_buffer<sl::constant_type<buffer_config, BufferConfig>, sl::size_constant_type<CommandGroupCount>>::
	try_emplace_back(Args&&... args)
	noexcept(sl::traits::is_noexcept_constructible_from_v<T, Args&&...>)
	requires(sl::traits::is_constructible_from_v<T, Args&&...>) {
		const sl::size_t old_size = this->size_bytes();
		RESULT_VERIFY(this->try_resize(old_size + sizeof(T)));

		return this->template emplace_to<T>(old_size, sl::forward<Args>(args)...);
	}
}

namespace acma::vk::generic {
    template<buffer_config BufferConfig, sl::size_t CommandGroupCount>
	requires(BufferConfig.memory != memory_policy::gpu_local)
	template<typename T, typename U>
    constexpr result<T*>
		resizable_gpu_buffer<sl::constant_type<buffer_config, BufferConfig>, sl::size_constant_type<CommandGroupCount>>::
	push_to(sl::uoffset_t dst_offset, U&& u)
	noexcept(sl::traits::is_noexcept_constructible_from_v<T, U&&>) {
		if(!this->last_clear_frame.matches(this->render_proc_core().frame_count()))
			return errc::buffer_needs_changes_applied; //TODO

		std::byte* dst = this->data();
		//TODO: wrap under unique_placement_ptr/unique_placement_ref so that non-trivial types are properly destructed
		T* ptr = new (dst + dst_offset) T(sl::forward<U>(u));
		return ptr;
	}

    template<buffer_config BufferConfig, sl::size_t CommandGroupCount>
	requires(BufferConfig.memory != memory_policy::gpu_local)
	template<typename T, typename... Args>
    constexpr result<T*>
		resizable_gpu_buffer<sl::constant_type<buffer_config, BufferConfig>, sl::size_constant_type<CommandGroupCount>>::
	emplace_to(sl::uoffset_t dst_offset, Args&&... args)
	noexcept(sl::traits::is_noexcept_constructible_from_v<T, Args&&...>) {
		if(!this->last_clear_frame.matches(this->render_proc_core().frame_count()))
			return errc::buffer_needs_changes_applied; //TODO

		std::byte* dst = this->data();
		//TODO: wrap under unique_placement_ptr/unique_placement_ref so that non-trivial types are properly destructed
		T* ptr = new (dst + dst_offset) T(sl::forward<Args>(args)...);
		return ptr;
	}
}
