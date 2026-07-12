#pragma once
#include "sirius/vulkan/memory/resizable_gpu_image_array.fwd.hpp"

#include <vector>
#include <streamline/functional/functor/invoke_each_result.hpp>

#include "sirius/vulkan/memory/image.hpp"
#include "sirius/graphics/core/texture_view.hpp"



namespace acma::vk::generic {
	template<sl::size_t CommandGroupCount>
	class resizable_gpu_image_array<sl::size_constant_type<CommandGroupCount>> : public std::vector<vk::image> {
	public:
		constexpr resizable_gpu_image_array() noexcept = delete;
		constexpr resizable_gpu_image_array(render_process_core<CommandGroupCount>& proc_core) noexcept :
			std::vector<vk::image>(),
			proc_ptr(std::addressof(proc_core)) {}
	private:
		template<typename> friend struct ::acma::impl::make;
		template<typename, auto> friend struct ::sl::functor::invoke_each_result;


	public:
		constexpr render_process_core<CommandGroupCount> const& render_proc_core() const noexcept { return *proc_ptr; }
		constexpr render_process_core<CommandGroupCount>      & render_proc_core()       noexcept { return *proc_ptr; }

	public:
		template<sl::traits::specialization_of<vk::generic::resizable_gpu_buffer> BufferT>
		constexpr result<void> emplace_back(BufferT const& texture_data_buffer, texture_info info, VkImageAspectFlags image_aspects = VK_IMAGE_ASPECT_COLOR_BIT) noexcept;


	private:
		sl::reference_ptr<render_process_core<CommandGroupCount>> proc_ptr;
	};
}

#include "sirius/vulkan/memory/resizable_gpu_image_array.inl"
