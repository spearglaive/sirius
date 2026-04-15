#pragma once
#include <memory>

#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/device/physical_device.hpp"
#include "sirius/vulkan/memory/allocator.hpp"

namespace acma::impl {
	//This class exists because these pointers need to be the first base class of render process
	//This is so that it gets destroyed after all the buffers and asset heaps
	class render_process_pointers {
	protected:
		vk::physical_device* phys_device_ptr; //This doesn't actually need to be here because its a reference
		std::shared_ptr<vk::logical_device> logi_device_ptr;
		vk::allocator_shared_handle alloc_ptr;
	};
}