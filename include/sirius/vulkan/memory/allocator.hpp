#pragma once
#include <memory>
#include <streamline/functional/functor/generic_stateless.hpp>
#include <streamline/memory/unique_ptr.hpp>
#include <streamline/metaprogramming/type_modifiers.hpp>

#include "sirius/vulkan/memory/allocator.fwd.hpp"


namespace acma::vk {
	using allocator_handle_t = VmaAllocator;
}

namespace acma::vk {
	struct allocator {
		constexpr operator allocator_handle_t() const noexcept { return smart_handle.get(); }
	public:
		sl::unique_ptr<sl::remove_pointer_t<allocator_handle_t>, sl::functor::generic_stateless<vmaDestroyAllocator>> smart_handle;
	};
}