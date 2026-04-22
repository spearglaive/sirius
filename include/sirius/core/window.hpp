#pragma once
#include <cstddef>
#include <functional>
#include <memory>
#include <string_view>
#include <streamline/functional/functor/generic_stateless.hpp>
#include <streamline/memory/unique_ptr.hpp>

#include "sirius/core/render_instance.fwd.hpp"
#include "sirius/core/window.fwd.hpp"
#include "sirius/arith/size.hpp"
#include "sirius/input/code.hpp"
#include "sirius/core/error.hpp"
#include "sirius/input/event_function.hpp"
#include "sirius/input/info.hpp"
#include "sirius/vulkan/display/surface.hpp"
#include "sirius/vulkan/display/swap_chain.hpp"
#include "sirius/vulkan/display/depth_image.hpp"
#include "sirius/vulkan/memory/allocator.hpp"


namespace acma {
    class SIRIUS_API window {
	public:
		template<typename T>
		friend struct ::acma::impl::make;
	public:
        constexpr window() noexcept :
			input_info_ptr(new input::info{.category_flags = static_cast<input::category_flags_t>(0b1) << input::category::system}),
			window_handle(), _surface{}, _swap_chain{}, _depth_image{}, _size{} {}
	public:
		result<void> initialize(
			sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
			sl::reference_ptr<const vk::logical_device> logi_device_ptr, 
			sl::reference_ptr<const vk::physical_device> phys_device_ptr,
			sl::reference_ptr<const vk::allocator> allocator
		) noexcept;
		
	public:
		result<bool> verify_swap_chain(
			VkResult fn_result, 
			sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
			sl::reference_ptr<const vk::logical_device> logi_device_ptr, 
			sl::reference_ptr<const vk::physical_device> phys_device_ptr,
			sl::reference_ptr<const vk::allocator> allocator,
			bool even_if_suboptimal
		) noexcept;

	public:
		constexpr sl::reference_ptr<const vk::surface    > surface_ptr    () const& noexcept { return {std::addressof(_surface    )}; }
		constexpr sl::reference_ptr<const vk::swap_chain > swap_chain_ptr () const& noexcept { return {std::addressof(_swap_chain )}; }
		constexpr sl::reference_ptr<const vk::depth_image> depth_image_ptr() const& noexcept { return {std::addressof(_depth_image)}; }
		
		constexpr extent2 screen_size() const noexcept { return _size; }
    
	public:
        constexpr auto&& current_input_categories(this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.input_info_ptr->category_flags); }
        constexpr auto&& input_active_bindings   (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.input_info_ptr->active_bindings); }
        constexpr auto&& input_inactive_bindings (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.input_info_ptr->inactive_bindings); }
        constexpr auto&& input_event_functions   (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.input_info_ptr->event_fns); }
        constexpr auto&& text_input_function     (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.input_info_ptr->text_input_fn); }
        constexpr auto&& input_modifier_flags    (this auto&& self) noexcept { return sl::forward_like<decltype(self)>(self.input_info_ptr->modifier_flags); }

    private:
        static void process_input(GLFWwindow* window_ptr, input::code_t code, bool pressed, input::mouse_aux_t mouse_aux_data) noexcept;
    private:
        static void kb_key_input(GLFWwindow* window_ptr, int key, int scancode, int action, int mods) noexcept;
        static void kb_text_input(GLFWwindow* window_ptr, unsigned int codepoint) noexcept; //not tied to a category
        static void mouse_move(GLFWwindow* window_ptr, double x, double y) noexcept; //can't be a modifier (no release either)
        static void mouse_button_input(GLFWwindow* window_ptr, int button, int action, int mods) noexcept;
        static void mouse_scroll(GLFWwindow* window_ptr, double x, double y) noexcept; //can't be a modifier (no release either)

	private:
    	template<typename TimelineT, auto BufferConfigs, auto AssetHeapConfigs>
		requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
		friend class render_instance;
		
	protected:
        sl::unique_ptr<input::info> input_info_ptr;
        sl::unique_ptr<GLFWwindow, sl::functor::generic_stateless<glfwDestroyWindow>> window_handle;
    private:
		vk::surface _surface;
		vk::swap_chain _swap_chain;
		vk::depth_image _depth_image;
		extent2 _size;

    };
}




namespace acma::impl {
	template<>
    struct make<window> {
		SIRIUS_API result<window> operator()(
			acma::sz2u32 size,
			std::string_view title,
			sl::in_place_adl_tag_type<window> = sl::in_place_adl_tag<window>
		) const noexcept;
	};
}

//#include "sirius/core/window.inl"