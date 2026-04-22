#include "sirius/vulkan/device/physical_device.hpp"

#include <cstring>
#include <streamline/functional/functor/construct_using.hpp>
#include <streamline/universal/make.hpp>
#include <streamline/algorithm/minmax.hpp>

#include "sirius/core/window.hpp"
#include "sirius/vulkan/device/queue_family_info.hpp"


namespace acma::impl {
	result<vk::physical_device>
		make<vk::physical_device>::
	operator()(
		VkPhysicalDevice device_handle,
		sl::in_place_adl_tag_type<vk::physical_device>
	) const noexcept {
        //Get device features and properties
		//VkPhysicalDeviceDescriptorHeapPropertiesEXT device_descriptor_heap_properties{
		//	.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_PROPERTIES_EXT,
		//};
        VkPhysicalDeviceProperties2 device_properties{
	    	.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
			//.pNext = &device_descriptor_heap_properties,
		};
        VkPhysicalDeviceFeatures device_features;
        sl::invoke(vkGetPhysicalDeviceProperties2, device_handle, &device_properties);
        sl::invoke(vkGetPhysicalDeviceFeatures, device_handle, &device_features);
		
		

        //Get device extensions
        vk::extensions_t device_extensions{};
        {
        std::uint32_t extension_count;
        __D2D_VULKAN_VERIFY(sl::invoke(vkEnumerateDeviceExtensionProperties, device_handle, nullptr, &extension_count, nullptr));
    
        std::vector<VkExtensionProperties> available_extensions(extension_count);
        __D2D_VULKAN_VERIFY(sl::invoke(vkEnumerateDeviceExtensionProperties, device_handle, nullptr, &extension_count, available_extensions.data()));
    
        //TODO replace with lookup table
        for (std::size_t i = 0; i < vk::extension::num_extensions; ++i){
            for (const auto& ext : available_extensions) {
                if(std::memcmp(ext.extensionName, vk::extension::name[i].data(), vk::extension::name[i].size()) == 0) {
                    device_extensions[i] = true;
                    goto next_extension;
                }
            }
            next_extension:;
        }
        }

        //Create device info
       	return vk::physical_device{
			.handle = device_handle,
            .name = device_properties.properties.deviceName,
            .type = static_cast<vk::device_type>(device_properties.properties.deviceType),
            .extensions = device_extensions,
            .features = std::bit_cast<vk::features_t>(device_features),
            .limits = device_properties.properties.limits,
			.descriptor_count_limits{{
				device_properties.properties.limits.maxDescriptorSetSamplers,

				device_properties.properties.limits.maxDescriptorSetSampledImages,
				device_properties.properties.limits.maxDescriptorSetStorageImages,
			}},
			.per_stage_descriptor_count_limits{{
				device_properties.properties.limits.maxPerStageDescriptorSamplers,
				
				device_properties.properties.limits.maxPerStageDescriptorSampledImages,
				device_properties.properties.limits.maxPerStageDescriptorStorageImages,
			}},

			.max_push_data_bytes = 0,//device_descriptor_heap_properties.maxPushDataSize,
			.asset_group_infos{{
				// {device_descriptor_heap_properties.samplerDescriptorSize, device_descriptor_heap_properties.samplerDescriptorAlignment},
				// {device_descriptor_heap_properties.imageDescriptorSize, device_descriptor_heap_properties.imageDescriptorAlignment},
				// {device_descriptor_heap_properties.bufferDescriptorSize, device_descriptor_heap_properties.bufferDescriptorAlignment},
			}},
			.descriptor_heap_infos{{
				// {
				// 	device_descriptor_heap_properties.minSamplerHeapReservedRange,
				// 	device_descriptor_heap_properties.samplerHeapAlignment,
				// 	device_descriptor_heap_properties.maxSamplerHeapSize
				// },
				// {
				// 	device_descriptor_heap_properties.minResourceHeapReservedRange,
				// 	device_descriptor_heap_properties.resourceHeapAlignment,
				// 	device_descriptor_heap_properties.maxResourceHeapSize
				// },
			}},
            .queue_family_infos = sl::universal::make<sl::array<command_family::num_families, vk::queue_family_info>>(
				sl::in_place_tag,
				false, 
				vk::impl::nidx,
				0
			),
			.max_queue_count = 0,
        };
	}
}


namespace acma::vk {
	result<void> physical_device::initialize_queues(bool prefer_synchronous_rendering, bool window_capability) noexcept {
        //Get device queue family indicies
		sl::uint32_t device_max_queue_count = 0;
        auto device_queue_family_infos = sl::universal::make<sl::array<command_family::num_families, queue_family_info>>(
			sl::in_place_tag,
			false, 
			impl::nidx,
			0
		);
        
		{
        std::uint32_t family_count = 0;
        sl::invoke(vkGetPhysicalDeviceQueueFamilyProperties, handle, &family_count, nullptr);

        std::vector<VkQueueFamilyProperties> families(family_count);
        sl::invoke(vkGetPhysicalDeviceQueueFamilyProperties, handle, &family_count, families.data());

        constexpr std::array<VkQueueFlagBits, command_family::num_distinct_families> flag_bit = {
            VK_QUEUE_GRAPHICS_BIT,
			VK_QUEUE_COMPUTE_BIT,
			VK_QUEUE_TRANSFER_BIT,
        }; 

		//Create dummy window
		window dummy_window{};
		if(window_capability)
			RESULT_TRY_MOVE(dummy_window, make<window>(acma::sz2u32{1280, 720}, "dummy"));

        for(std::uint32_t idx = 0; idx < family_count; ++idx) {
            VkBool32 supports_present = false;
			const sl::uint32_t queue_count = families[idx].queueCount;
			device_max_queue_count = sl::algo::max(max_queue_count, queue_count);
			if(window_capability) {
            	sl::invoke(vkGetPhysicalDeviceSurfaceSupportKHR, handle, idx, *dummy_window.surface_ptr(), &supports_present);
				if(supports_present && (device_queue_family_infos[command_family::present].index == impl::nidx || !prefer_synchronous_rendering)) 
					device_queue_family_infos[command_family::present] = queue_family_info{true, idx, queue_count};
			}

            for(std::size_t family_id = 0; family_id < command_family::num_distinct_families; ++family_id) {    
				if(!(families[idx].queueFlags & flag_bit[family_id])) continue;
				if(device_queue_family_infos[family_id].index != impl::nidx && prefer_synchronous_rendering) continue;
				device_queue_family_infos[family_id] = queue_family_info{static_cast<bool>(supports_present), idx, queue_count};
            }
        }
		}

		//Check for queue family support
		//TODO: only check for queues that we actually need (based on graphics timeline)?
		const sl::size_t num_queue_families_to_verify = command_family::num_distinct_families + static_cast<sl::size_t>(window_capability);
		for(std::size_t i = 0; i < num_queue_families_to_verify; ++i)
			if(device_queue_family_infos[i].index == impl::nidx)
				return static_cast<errc>(error::device_lacks_necessary_queue_base + i);

		queue_family_infos = device_queue_family_infos;
		max_queue_count = device_max_queue_count;
		return {};
	}
}



namespace acma::vk {
    template<> typename device_query_traits<device_query::surface_capabilites>::return_type physical_device::query<device_query::surface_capabilites>(surface const& s) const noexcept {
        VkSurfaceCapabilitiesKHR device_surface_capabilities;
        sl::invoke(vkGetPhysicalDeviceSurfaceCapabilitiesKHR, handle, s, &device_surface_capabilities);
        return device_surface_capabilities;
    }

    template<> typename device_query_traits<device_query::display_formats>::return_type physical_device::query<device_query::display_formats>(surface const& s) const noexcept {
        std::set<display_format> formats;
        
        std::uint32_t format_count;
        sl::invoke(vkGetPhysicalDeviceSurfaceFormatsKHR, handle, s, &format_count, nullptr);

        std::vector<VkSurfaceFormatKHR> surface_formats;
        if (format_count != 0) {
            surface_formats.resize(format_count);
            sl::invoke(vkGetPhysicalDeviceSurfaceFormatsKHR, handle, s, &format_count, surface_formats.data());
        }

        //TODO replace with lookup table
        for(VkSurfaceFormatKHR f : surface_formats)
            formats.emplace(pixel_formats.find(f.format)->second, color_spaces.find(f.colorSpace)->second);

        //TODO replace with format to bool lookup table?
        return formats;
    }

    template<> typename device_query_traits<device_query::present_modes>::return_type physical_device::query<device_query::present_modes>(surface const& s) const noexcept {
        typename device_query_traits<device_query::present_modes>::return_type device_present_modes;
        
        uint32_t present_mode_count;
        sl::invoke(vkGetPhysicalDeviceSurfacePresentModesKHR, handle, s, &present_mode_count, nullptr);

        std::vector<VkPresentModeKHR> supported_present_modes;
        if (present_mode_count != 0) {
            supported_present_modes.resize(present_mode_count);
            sl::invoke(vkGetPhysicalDeviceSurfacePresentModesKHR, handle, s, &present_mode_count, supported_present_modes.data());
        }

        for(VkPresentModeKHR p : supported_present_modes)
            if(p <= VK_PRESENT_MODE_FIFO_RELAXED_KHR)
                device_present_modes[p] = true;
        
        return device_present_modes;
    }
}