#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/device/physical_device.hpp"
#include "sirius/vulkan/core/vulkan.hpp"


namespace acma::impl {
	result<vk::logical_device>
		make<vk::logical_device>::
	operator()(
		sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
		sl::reference_ptr<const vk::physical_device> associated_phys_device_ptr,
		bool windowing,
		sl::in_place_adl_tag_type<vk::logical_device>
	) const noexcept {
		const sl::size_t queue_create_info_count = command_family::num_distinct_families + static_cast<sl::size_t>(windowing);
		if(associated_phys_device_ptr->max_queue_count == 0)
			return errc::vulkan_device_lost;

        //Create desired queues for each queue family
        std::array<VkDeviceQueueCreateInfo, command_family::num_families> queue_create_infos{};
		std::unordered_map<sl::uint32_t, bool> duplicate_index{};
		std::unique_ptr<float[]> priorities = std::make_unique<float[]>(associated_phys_device_ptr->max_queue_count);
		priorities[0] = 1.f;

		sl::uint32_t queue_create_count = 0;
        for(std::size_t i = 0; i < queue_create_info_count; ++i) {
			const sl::uint32_t queue_family_index = associated_phys_device_ptr->queue_family_infos[i].index;
			if(duplicate_index[queue_family_index])
				continue;
            queue_create_infos[i] = VkDeviceQueueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = queue_family_index,
                .queueCount = associated_phys_device_ptr->queue_family_infos[i].queue_count,
                .pQueuePriorities = priorities.get(),
            };
			++queue_create_count;
			duplicate_index[queue_family_index] = true;
        }

        //Set desired features
		// VkPhysicalDeviceDescriptorHeapFeaturesEXT desired_descriptor_heap_features{
			// .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT,
			// .pNext = nullptr,
			// .descriptorHeap = VK_TRUE,
		// };
		// VkPhysicalDeviceMaintenance5FeaturesKHR desired_maintenance_5_features{
			// .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR,
			// .pNext = &desired_descriptor_heap_features,
			// .maintenance5 = VK_TRUE,
		// };
        VkPhysicalDeviceVulkan13Features desired_1_3_features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.pNext = nullptr,//&desired_maintenance_5_features,
            .synchronization2 = VK_TRUE,
			.dynamicRendering = VK_TRUE,
			.maintenance4 = VK_TRUE,
        };
        VkPhysicalDeviceVulkan12Features desired_1_2_features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &desired_1_3_features,
			.drawIndirectCount = VK_TRUE,
            .descriptorIndexing = VK_TRUE,
            .shaderUniformBufferArrayNonUniformIndexing = VK_TRUE,
            .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
            .shaderStorageImageArrayNonUniformIndexing = VK_TRUE,
			.descriptorBindingVariableDescriptorCount = VK_TRUE,
            .runtimeDescriptorArray = VK_TRUE,
			.scalarBlockLayout = VK_TRUE,
            .timelineSemaphore = VK_TRUE,
			.bufferDeviceAddress = VK_TRUE,
        };
        VkPhysicalDeviceVulkan11Features desired_1_1_features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext = &desired_1_2_features,
			.shaderDrawParameters = VK_TRUE,
        };
        VkPhysicalDeviceFeatures desired_base_features {
            .multiDrawIndirect = VK_TRUE,
            .samplerAnisotropy = VK_TRUE,
        };
		VkPhysicalDeviceFeatures2 desired_features {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &desired_1_1_features,
			.features = desired_base_features,
		};

        //Set extensions
        //TODO improve with lookup table?
        std::vector<const char*> enabled_extensions{};
        for(std::size_t i = 0; i < associated_phys_device_ptr->extensions.size(); ++i)
            if(associated_phys_device_ptr->extensions[i] && (windowing || i != vk::extension::swap_chain))
                enabled_extensions.push_back(vk::extension::name[i].data());

        //Create logical device
        VkDeviceCreateInfo device_create_info{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &desired_features,
            .queueCreateInfoCount = queue_create_count,
            .pQueueCreateInfos = queue_create_infos.data(),
            .enabledLayerCount = 0,
            .enabledExtensionCount = static_cast<std::uint32_t>(enabled_extensions.size()),
            .ppEnabledExtensionNames = enabled_extensions.data(),
            .pEnabledFeatures = VK_NULL_HANDLE,
        };

        vk::logical_device ret{{{vulkan_fns_ptr->vkDestroyDevice}}, {}};
        __D2D_VULKAN_VERIFY(sl::invoke(vkCreateDevice, associated_phys_device_ptr->handle, &device_create_info, nullptr, &ret));


        //Create queues
        for(std::size_t i = 0; i < queue_create_info_count; ++i) {
			const sl::uint32_t queue_count = associated_phys_device_ptr->queue_family_infos[i].queue_count;
			ret.queues[i].resize(queue_count);
            for(sl::size_t j = 0; j < queue_count; ++j)
				sl::invoke(
					vulkan_fns_ptr->vkGetDeviceQueue,
					ret, static_cast<sl::uint32_t>(associated_phys_device_ptr->queue_family_infos[i].index), j, &ret.queues[i][j]
				);
		}

		return ret;
	};
}

namespace acma::vk {
	// void logical_device::initialize(
	// 	sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
	// 	sl::reference_ptr<const vk::physical_device> associated_phys_device_ptr
	// ) noexcept {
	// }
}