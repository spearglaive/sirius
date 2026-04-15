#pragma once
#include <memory>
#include <streamline/functional/functor/generic_stateless.hpp>
#include <streamline/memory/unique_ptr.hpp>
#include <streamline/metaprogramming/type_modifiers.hpp>

#include "sirius/vulkan/memory/allocator.fwd.hpp"


namespace acma::vk {
	using allocator = sl::remove_pointer_t<VmaAllocator>;
}

namespace acma::vk {
	using allocator_shared_handle = std::shared_ptr<vk::allocator>; //sl::unique_ptr<vk::allocator, sl::functor::generic_stateless<vmaDestroyAllocator>>;
}