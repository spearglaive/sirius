#pragma once
#include "sirius/vulkan/memory/resizable_gpu_image_array.hpp"


namespace acma::vk::generic {
	template<sl::size_t CommandGroupCount>
	template<sl::traits::specialization_of<vk::generic::resizable_gpu_buffer> BufferT>
	constexpr result<void>	resizable_gpu_image_array<sl::size_constant_type<CommandGroupCount>>::
	emplace_back(BufferT const& texture_data_buffer, texture_info info, VkImageAspectFlags image_aspects) noexcept {

		if(texture_data_buffer.size() == 0)
			return {};

		const sl::size_t old_image_count = this->size();

		RESULT_VERIFY_UNSCOPED(::acma::make<image>(
			render_proc_core().allocator_ptr(),
			static_cast<VkImageCreateInfo>(info),
			image_aspects
		), img_result);
		this->push_back(*sl::move(img_result));

		return gpu_copy<CommandGroupCount, BufferT>(render_proc_core(),
			{this->data() + old_image_count},
			{std::addressof(texture_data_buffer)},
			{&info},
			1
		);
	}
}
