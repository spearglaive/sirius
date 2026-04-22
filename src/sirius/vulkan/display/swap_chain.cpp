#include "sirius/vulkan/display/swap_chain.hpp"

#include "sirius/core/error.hpp"
#include "sirius/vulkan/device/device_query.hpp"
#include "sirius/vulkan/display/color_space.hpp"
#include "sirius/vulkan/display/display_format.hpp"
#include "sirius/vulkan/display/image_view.hpp"
#include "sirius/core/make.hpp"
#include "sirius/vulkan/device/physical_device.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/display/pixel_format.hpp"
#include "sirius/vulkan/display/present_mode.hpp"
#include "sirius/vulkan/core/vulkan.hpp"


namespace acma::vk {
	 result<void> swap_chain::reset(
		sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
		sl::reference_ptr<const physical_device> phys_device_ptr,
		sl::reference_ptr<const logical_device> logi_device_ptr,
		surface& window_surface,
		GLFWwindow& window_instance,
		std::span<const pixel_format_info> pixel_format_priority,
		color_space_info color_space,
		std::span<const present_mode> present_mode_priority
	) noexcept {
        VkSurfaceCapabilitiesKHR device_capabilities = phys_device_ptr->query<device_query::surface_capabilites>(window_surface);

        //Create swap extent
        if(device_capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>().max())
            _extent = {device_capabilities.currentExtent.width, device_capabilities.currentExtent.height};
        else {
            int width = 0, height = 0;
            glfwGetFramebufferSize(&window_instance, &width, &height);
            __D2D_GLFW_VERIFY((width != 0 && height != 0));
            _extent = {
                std::clamp(static_cast<uint32_t>(width), device_capabilities.minImageExtent.width, device_capabilities.maxImageExtent.width),
                std::clamp(static_cast<uint32_t>(height), device_capabilities.minImageExtent.height, device_capabilities.maxImageExtent.height)
            };
        }



        //Check display format support
        auto display_formats = phys_device_ptr->query<device_query::display_formats>(window_surface);
        for(std::size_t i = 0; i < pixel_format_priority.size(); ++i) {
            const auto it = display_formats.find({pixel_format_priority[i], color_space});
            if(it != display_formats.end()) {
                _display_format = *it;
                goto found_display_format;
            }
        }
        _display_format = *display_formats.begin();
        //return error::device_lacks_display_format;
    found_display_format:

        //Check present mode support
        auto present_modes = phys_device_ptr->query<device_query::present_modes>(window_surface);
        for(present_mode p : present_mode_priority) {
            if(present_modes[static_cast<std::size_t>(p)]) {
                _present_mode = p;
                goto found_present_mode;
            }
        }
        _present_mode = vk::present_mode::fifo;
    found_present_mode:


        //Create swap chain
        {
        std::uint32_t image_count = device_capabilities.minImageCount + 1;
        if(device_capabilities.maxImageCount > 0)
            image_count = std::min(device_capabilities.maxImageCount, image_count);

        const VkSwapchainCreateInfoKHR swap_chain_create_info{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = window_surface,
            .minImageCount = image_count,
            .imageFormat = _display_format.pixel_format.id,
            .imageColorSpace = _display_format.color_space.id,
            .imageExtent = static_cast<VkExtent2D>(_extent),
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr,
            .preTransform = device_capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = static_cast<VkPresentModeKHR>(_present_mode),
            .clipped = VK_TRUE,
            .oldSwapchain = smart_handle.get(),
        };

        __D2D_VULKAN_VERIFY(sl::invoke(vulkan_fns_ptr->vkCreateSwapchainKHR, *logi_device_ptr, &swap_chain_create_info, nullptr, &smart_handle.get()));
        }

        //Get swap chain images
        {
        _image_count = 0;
        __D2D_VULKAN_VERIFY(sl::invoke(vulkan_fns_ptr->vkGetSwapchainImagesKHR, *logi_device_ptr, smart_handle.get(), &_image_count, nullptr));
        _images = std::make_unique_for_overwrite<VkImage[]>(_image_count);
        __D2D_VULKAN_VERIFY(sl::invoke(vulkan_fns_ptr->vkGetSwapchainImagesKHR, *logi_device_ptr, smart_handle.get(), &_image_count, _images.get()));
        }

        //Create swap chain image views
        _image_views = std::make_unique_for_overwrite<image_view[]>(_image_count);
        for (size_t i = 0; i < _image_count; i++) {
        	const VkImageViewCreateInfo image_view_create_info{
        	    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        	    .image = _images[i],
        	    .viewType = VK_IMAGE_VIEW_TYPE_2D,
        	    .format = _display_format.pixel_format.id,
        	    .components{},
        	    .subresourceRange{
        	        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        	        .baseMipLevel = 0,
        	        .levelCount = 1,
        	        .baseArrayLayer = 0,
        	        .layerCount = 1,
        	    },
        	};
            RESULT_TRY_MOVE(_image_views[i], make<image_view>(
				vulkan_fns_ptr,
				logi_device_ptr,
				image_view_create_info
			));
        }

		return {};
    }
}


namespace acma::impl {
	result<vk::swap_chain>
		make<vk::swap_chain>::
	operator()(
		sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
		sl::reference_ptr<const vk::physical_device> phys_device_ptr,
		sl::reference_ptr<const vk::logical_device> logi_device_ptr,
		vk::surface& window_surface,
		GLFWwindow& window_instance,
		std::span<const vk::pixel_format_info> pixel_format_priority,
		vk::color_space_info color_space,
		std::span<const vk::present_mode> present_mode_priority,
		sl::in_place_adl_tag_type<vk::swap_chain>
	) const noexcept {
		vk::swap_chain ret{{{vulkan_fns_ptr->vkDestroySwapchainKHR, logi_device_ptr}}, {}, {}, {}, {}, {}, {}};
		RESULT_VERIFY(ret.reset(vulkan_fns_ptr, phys_device_ptr, logi_device_ptr, window_surface, window_instance, pixel_format_priority, color_space, present_mode_priority));
		return ret;
	}
}