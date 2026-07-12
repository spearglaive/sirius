#pragma once
#include <streamline/containers/generic_container_template_args.def.hpp>
#include <streamline/metaprogramming/constant.hpp>
#include <streamline/metaprogramming/type_modifiers.hpp>


namespace acma::vk::generic {
	template<SL_GENERIC_CONTAINER_TEMPLATE_ARGS()>
	class resizable_gpu_image_array;
}


namespace acma::vk {
	template<typename RenderProcessT>
	using resizable_gpu_image_array = generic::resizable_gpu_image_array<
		sl::size_constant_type<sl::remove_cvref_t<RenderProcessT>::command_buffer_count>
	>;
}
