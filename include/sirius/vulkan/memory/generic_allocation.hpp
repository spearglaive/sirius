#pragma once
#include <streamline/functional/functor/generic_stateful.hpp>

#include "sirius/vulkan/core/vulkan.hpp"
#include "sirius/vulkan/memory/allocator.hpp"

namespace acma::vk {
	using allocation_t = sl::remove_pointer_t<VmaAllocation>;
	using allocation_handle_t = allocation_t*;
	using allocation_creation_info_t = VmaAllocationCreateInfo;
	using allocation_info_t = VmaAllocationInfo;
}

namespace acma::vk {
	template<typename HandleT, typename CreationInfoT>
	struct generic_allocation {
		HandleT handle;
		CreationInfoT creation_info;
		allocation_handle_t allocation_handle;
		allocation_creation_info_t allocation_creation_info;
		allocation_info_t allocation_info;
	public:
		using handle_type = HandleT;
	};
}


namespace acma::vk::impl {
	template<typename AllocationT, void(*DeleteFn)(allocator*, typename AllocationT::handle_type, allocation_handle_t)>
	struct allocation_deleter{
		constexpr allocation_deleter() noexcept = default;
		constexpr allocation_deleter(allocator_shared_handle const& allocator) noexcept :
			allocator_ptr(allocator) {}
	public:
		constexpr void operator()(AllocationT* allocation_ptr) noexcept {
			if(!allocation_ptr->handle) {
				if(!allocation_ptr->allocation_handle)
					return;
				return vmaFreeMemory(allocator_ptr.get(), allocation_ptr->allocation_handle);
			}
			return DeleteFn(
				allocator_ptr.get(),
				allocation_ptr->handle,
				allocation_ptr->allocation_handle
			);
		}

	private:
		allocator_shared_handle allocator_ptr;
	};
}
