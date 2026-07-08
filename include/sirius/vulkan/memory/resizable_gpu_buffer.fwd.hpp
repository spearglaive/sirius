#pragma once
#include <streamline/metaprogramming/constant.hpp>
#include <streamline/numeric/int.hpp>
#include <streamline/containers/generic_container_template_args.def.hpp>

#include "sirius/core/buffer_config.hpp"
#include "sirius/core/buffer_key_t.hpp"



namespace acma::vk::generic {
	template<SL_GENERIC_CONTAINER_TEMPLATE_ARGS()>
	class resizable_gpu_buffer;
}


namespace acma::vk {
	template<buffer_config Config, typename RenderProcessT>
	using resizable_gpu_buffer = generic::resizable_gpu_buffer<
		sl::constant_type<buffer_config, Config>,
		sl::size_constant_type<sl::remove_cvref_t<RenderProcessT>::command_buffer_count>
	>;
}
