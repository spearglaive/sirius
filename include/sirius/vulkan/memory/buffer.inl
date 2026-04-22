#pragma once
#include "sirius/vulkan/memory/buffer.hpp"

#include <numeric>
#include <streamline/algorithm/aligned_to.hpp>
#include <streamline/functional/functor/address_of.hpp>
#include <streamline/functional/invoke.hpp>

#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/error.hpp"
#include "sirius/core/render_process.fwd.hpp"


namespace acma::vk::impl {
	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
    result<void>    buffer_base<K, BufferConfigs, RenderProcessT>::
	initialize() noexcept {
		RenderProcessT const& proc = static_cast<RenderProcessT const&>(*this);
		for(sl::index_t i = 0; i < allocation_count; ++i)
			RESULT_TRY_MOVE(buff_alloc_ptrs[i], gpu_allocate<BufferConfigs[K]>(proc, sl::constant<buffer_config, config>));
        return {};
    }
}



namespace acma::vk::impl {
	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
    constexpr sl::index_t    buffer_properties<K, BufferConfigs, RenderProcessT>::
	allocation_index() const noexcept {
		return (static_cast<RenderProcessT const&>(*this).frame_count()) % allocation_count;
	}
}



namespace acma::vk {
	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
	constexpr result<void>    impl::buffer_base<K, BufferConfigs, RenderProcessT>::
	reserve(sl::size_t new_capacity_bytes) noexcept {
		if(new_capacity_bytes <= this->capacity_bytes()) 
			return {};

		RenderProcessT& proc = static_cast<RenderProcessT&>(*this);
		const sl::index_t alloc_idx = this->allocation_index();
		if(new_capacity_bytes <= this->allocated_bytes()) {
			sl::invoke(proc.vulkan_functions_ptr()->vkDestroyBuffer, *proc.logical_device_ptr(), buff_alloc_ptrs[alloc_idx]->handle, nullptr);
			buff_alloc_ptrs[alloc_idx]->handle = VK_NULL_HANDLE;

			buffer_creation_info_t create_info = buff_alloc_ptrs[alloc_idx]->creation_info;
			create_info.size = new_capacity_bytes;

			__D2D_VULKAN_VERIFY(vmaCreateAliasingBuffer2(
				*proc.allocator_ptr(),
				buff_alloc_ptrs[alloc_idx]->allocation_handle,
				0,
				&create_info,
				&buff_alloc_ptrs[alloc_idx]->handle
			));
			buff_alloc_ptrs[alloc_idx]->creation_info = create_info;

			
			const VkBufferDeviceAddressInfo device_address_info{
				.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR,
				.buffer = buff_alloc_ptrs[alloc_idx]->handle
			};
			buff_alloc_ptrs[alloc_idx]->device_address = sl::invoke(proc.vulkan_functions_ptr()->vkGetBufferDeviceAddress, *proc.logical_device_ptr(), &device_address_info);
		}
		else {
			vk::buffer_allocation_unique_ptr old_alloc = std::move(buff_alloc_ptrs[alloc_idx]);
			RESULT_TRY_MOVE(buff_alloc_ptrs[alloc_idx], gpu_allocate_like(proc, old_alloc, new_capacity_bytes));
			RESULT_VERIFY(gpu_copy(
				proc, 
				buff_alloc_ptrs[alloc_idx], sl::constant<buffer_config, config>,
				old_alloc, sl::constant<buffer_config, config>,
				old_alloc->creation_info.size
			));
		}
		return {};
	}
}


namespace acma::vk {
	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
	constexpr void    impl::buffer_base<K, BufferConfigs, RenderProcessT>::
	clear() noexcept {
		this->data_bytes[this->allocation_index()] = 0;
	}
	

	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
	constexpr result<void>    impl::buffer_base<K, BufferConfigs, RenderProcessT>::
	resize(sl::size_t count_bytes) noexcept {
		RESULT_VERIFY(reserve(count_bytes));
		this->data_bytes[this->allocation_index()] = count_bytes;
		return {};
	}

	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
	constexpr result<void>    impl::buffer_base<K, BufferConfigs, RenderProcessT>::
	try_resize(sl::size_t count_bytes) noexcept {
		if(count_bytes > this->capacity_bytes())
			return errc::not_enough_memory;
		this->data_bytes[this->allocation_index()] = count_bytes;
		return {};
	}
}


namespace acma::vk {
	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
	requires(
		!(impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data)) &&
		impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
	)
	template<buffer_key_t DstK>
	constexpr result<void>    buffer<K, BufferConfigs, RenderProcessT>::
	upload_to(buffer<DstK, BufferConfigs, RenderProcessT>& dst, sl::uoffset_t dst_offset) noexcept {
		RESULT_VERIFY(dst.resize(dst_offset + this->size_bytes()));
		
		RESULT_VERIFY(gpu_copy(dst, *this, this->size_bytes(), dst_offset, 0));
		this->clear();
		return {};
	}

	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
	requires(
		!(impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data)) &&
		impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
	)
	template<buffer_key_t DstK>
	constexpr result<void>    buffer<K, BufferConfigs, RenderProcessT>::
	try_upload_to(buffer<DstK, BufferConfigs, RenderProcessT>& dst, sl::uoffset_t dst_offset) noexcept {
		RESULT_VERIFY(dst.try_resize(dst_offset + this->size_bytes()));
		
		RESULT_VERIFY(gpu_copy(dst, *this, this->size_bytes(), dst_offset, 0));
		this->clear();
		return {};
	}
}

namespace acma::vk {
	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
	requires(
		!(impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data)) &&
		impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
	)
	template<typename T>
	constexpr result<void>    buffer<K, BufferConfigs, RenderProcessT>::
	push_back(T&& t) 
	noexcept(sl::traits::is_noexcept_constructible_from_v<T, T&&>)
	requires(sl::traits::is_constructible_from_v<T, T&&> && config.memory != memory_policy::push_constant) {
		const sl::size_t old_size = this->size_bytes();
		RESULT_VERIFY(this->resize(old_size + sizeof(T)));
		
		return push_to(old_size, sl::forward<T>(t));
	}

	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
	requires(
		!(impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data)) &&
		impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
	)
	template<typename T>
	constexpr result<void>    buffer<K, BufferConfigs, RenderProcessT>::
	try_push_back(T&& t) 
	noexcept(sl::traits::is_noexcept_constructible_from_v<T, T&&>)
	requires(sl::traits::is_constructible_from_v<T, T&&>) {
		const sl::size_t old_size = this->size_bytes();
		RESULT_VERIFY(this->try_resize(old_size + sizeof(T)));
		
		return push_to(old_size, sl::forward<T>(t));
	}
}

namespace acma::vk {
	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
	requires(
		!(impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data)) &&
		impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
	)
	template<typename T, typename... Args>
	constexpr result<void>    buffer<K, BufferConfigs, RenderProcessT>::
	emplace_back(Args&&... args)
	noexcept(sl::traits::is_noexcept_constructible_from_v<T, Args&&...>)
	requires(sl::traits::is_constructible_from_v<T, Args&&...> && config.memory != memory_policy::push_constant) {
		const sl::size_t old_size = this->size_bytes();
		RESULT_VERIFY(this->resize(old_size + sizeof(T)));
		
		return emplace_to<T>(old_size, sl::forward<Args>(args)...);
	}

	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
	requires(
		!(impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data)) &&
		impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
	)
	template<typename T, typename... Args>
	constexpr result<void>    buffer<K, BufferConfigs, RenderProcessT>::
	try_emplace_back(Args&&... args)
	noexcept(sl::traits::is_noexcept_constructible_from_v<T, Args&&...>)
	requires(sl::traits::is_constructible_from_v<T, Args&&...>) {
		const sl::size_t old_size = this->size_bytes();
		RESULT_VERIFY(this->try_resize(old_size + sizeof(T)));

		return emplace_to<T>(old_size, sl::forward<Args>(args)...);
	}
}

namespace acma::vk {
    template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
	requires(
		!(impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data)) &&
		impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
	)
	template<typename T>
    constexpr result<void>
		buffer<K, BufferConfigs, RenderProcessT>::
	push_to(sl::uoffset_t dst_offset, T&& t) 
	noexcept(sl::traits::is_noexcept_constructible_from_v<T, T&&>) {
		std::byte* dst = this->data();
		new (dst + dst_offset) sl::remove_cvref_t<T>(sl::forward<T>(t));
		return {};
	}

    template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
	requires(
		!(impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data)) &&
		impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.memory != memory_policy::gpu_local
	)
	template<typename T, typename... Args>
    constexpr result<void>
		buffer<K, BufferConfigs, RenderProcessT>::
	emplace_to(sl::uoffset_t dst_offset, Args&&... args)
	noexcept(sl::traits::is_noexcept_constructible_from_v<T, Args&&...>) {
		std::byte* dst = this->data();
		new (dst + dst_offset) sl::remove_cvref_t<T>(sl::forward<Args>(args)...);
		return {};
	}
}



namespace acma::vk {
	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
	requires (
		static_cast<bool>(impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data))
	)
	constexpr void    buffer<K, BufferConfigs, RenderProcessT>::
	clear() noexcept {
		base_type::clear();
		texture_data_infos.clear();
	}
}

namespace acma::vk {
	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
	requires (
		static_cast<bool>(impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data))
	)
	constexpr result<void>    buffer<K, BufferConfigs, RenderProcessT>::
	push_back(texture_view t) noexcept {
		const sl::size_t old_size = this->size_bytes();
		RESULT_VERIFY(this->resize(old_size + t.bytes.size_bytes()));
		const sl::uoffset_t offset = texture_data_infos.empty() ? 0 : (texture_data_infos.back().offset + texture_data_infos.back().size);
		texture_data_infos.push_back(texture_data_info{t, offset, t.bytes.size_bytes()});
		
		std::memcpy(this->data() + offset, t.bytes.data(), t.bytes.size_bytes());
		return {};
	}

	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
	requires (
		static_cast<bool>(impl::buffer_base<K, BufferConfigs, RenderProcessT>::config.usage & (buffer_usage_policy::texture_data))
	)
	constexpr result<void>    buffer<K, BufferConfigs, RenderProcessT>::
	try_push_back(texture_view t) noexcept {
		const sl::size_t old_size = this->size_bytes();
		RESULT_VERIFY(this->try_resize(old_size + t.bytes.size_bytes()));
		const sl::uoffset_t offset = texture_data_infos.empty() ? 0 : (texture_data_infos.back().offset + texture_data_infos.back().size);
		texture_data_infos.push_back(texture_data_info{t, offset, t.bytes.size_bytes()});
		
		std::memcpy(this->data() + offset, t.bytes.data(), t.bytes.size_bytes());
		return {};
	}
}
