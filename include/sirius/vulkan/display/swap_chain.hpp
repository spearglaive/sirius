#pragma once
#include <vector>
#include <streamline/memory/reference_ptr.hpp>

#include <GLFW/glfw3.h>

#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/make.hpp"
#include "sirius/vulkan/core/mixin.hpp"
#include "sirius/vulkan/core/unique_vk_ptr.hpp"
#include "sirius/vulkan/display/color_space.hpp"
#include "sirius/vulkan/display/display_format.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/device/physical_device.hpp"
#include "sirius/vulkan/display/pixel_format.hpp"
#include "sirius/vulkan/display/present_mode.hpp"
#include "sirius/vulkan/display/surface.hpp"
#include "sirius/vulkan/display/image_view.hpp"
#include "sirius/arith/size.hpp"


namespace acma::vk {
    struct SIRIUS_API swap_chain : mixin<VkSwapchainKHR, PFN_vkDestroySwapchainKHR, logical_device> {
	public:
		constexpr static std::array<vk::pixel_format_info, 2> default_pixel_format_priority = {vk::pixel_formats.find(VK_FORMAT_B8G8R8A8_SRGB)->second, vk::pixel_formats.find(VK_FORMAT_B8G8R8A8_UNORM)->second};
		constexpr static vk::color_space_info default_color_space = vk::color_spaces.find(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)->second;
		constexpr static std::array<vk::present_mode, static_cast<std::size_t>(vk::present_mode::num_present_modes)> default_present_mode_priority{vk::present_mode::mailbox, vk::present_mode::fifo, vk::present_mode::fifo_relaxed, vk::present_mode::immediate}; 
	

	public:
		template<typename T>
		friend struct ::acma::impl::make;
	public:
		result<void> reset(
			sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
			sl::reference_ptr<const physical_device> phys_device_ptr,
			sl::reference_ptr<const logical_device> logi_deivce,
			surface& window_surface,
			GLFWwindow& window_instance,
			std::span<const pixel_format_info> pixel_format_priority = default_pixel_format_priority,
			color_space_info color_space = default_color_space,
			std::span<const present_mode> present_mode_priority = default_present_mode_priority
		) noexcept;

    
	public:
        constexpr auto&& extent     (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._extent); }
        constexpr auto&& image_count(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._image_count); }
        constexpr auto&& format     (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._display_format); }
        constexpr auto&& mode       (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._present_mode); }
        constexpr auto&& image_views(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._image_views); }
        constexpr auto&& images     (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self._images); }


    public:
        extent2 _extent;
        std::uint32_t _image_count;
        vk::display_format _display_format;
        vk::present_mode _present_mode;
        std::unique_ptr<image_view[]> _image_views;
        std::unique_ptr<VkImage[]> _images;
    };
}

namespace acma::impl {
	template<>
    struct make<vk::swap_chain> {
		SIRIUS_API result<vk::swap_chain> operator()(
			sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
			sl::reference_ptr<const vk::physical_device> phys_device_ptr,
			sl::reference_ptr<const vk::logical_device> logi_device_ptr,
			vk::surface& window_surface,
			GLFWwindow& window_instance,
			std::span<const vk::pixel_format_info> pixel_format_priority = vk::swap_chain::default_pixel_format_priority,
			vk::color_space_info color_space = vk::swap_chain::default_color_space,
			std::span<const vk::present_mode> present_mode_priority = vk::swap_chain::default_present_mode_priority,
			sl::in_place_adl_tag_type<vk::swap_chain> = sl::in_place_adl_tag<vk::swap_chain>
		) const noexcept;
	};
}