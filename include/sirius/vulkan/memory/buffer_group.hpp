#pragma once
#include "sirius/core/render_process.fwd.hpp"

#include <streamline/functional/functor/subscript.hpp>
#include <streamline/functional/functor/identity_index.hpp>
#include <streamline/functional/functor/generic_stateless.hpp>

#include "sirius/vulkan/memory/buffer.hpp"



namespace acma::vk::impl {
	template<typename, auto, typename>
	class buffer_group;
}


namespace acma::vk::impl {
	template<sl::size_t... Is, buffer_config_table<sizeof...(Is)> BufferConfigs, typename RenderProcessT>
	class buffer_group<sl::index_sequence_type<Is...>, BufferConfigs, RenderProcessT> : 
		public buffer<
			sl::universal::get<sl::first_constant>(*std::next(BufferConfigs.begin(), Is)),
			BufferConfigs,
			RenderProcessT
		>... 
	{};
}
