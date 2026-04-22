#pragma once
#include <memory>
#include <streamline/memory/unique_ptr.hpp>
#include <streamline/functional/functor/generic_stateless.hpp>

#include "sirius/vulkan/core/unique_vk_ptr.hpp"
#include "sirius/vulkan/core/command_pool.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/device/physical_device.hpp"
#include "sirius/vulkan/memory/allocator.hpp"

namespace acma::impl {
	//This class exists because these types need to be the first base class of render process
	//This is so that it gets destroyed after all the buffers and asset heaps
	//Theyre all smart pointers because these specific types also need to be stored
	class render_process_core {
	protected:
		sl::unique_ptr<vk::function_table> _vulkan_functions_ptr;
		sl::reference_ptr<vk::physical_device> _physical_device_ptr; //This is actually a reference, so it doesn't need to be a smart pointer
		sl::unique_ptr<vk::logical_device> _logical_device_ptr;
		sl::unique_ptr<vk::allocator> _allocator_ptr;
		sl::array<impl::command_pool_count, sl::unique_ptr<vk::command_pool>> _command_pool_ptrs;
	};
}