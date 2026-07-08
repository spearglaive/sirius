#pragma once
#include "sirius/core/render_process.fwd.hpp"

#include <streamline/functional/functor/subscript.hpp>
#include <streamline/functional/functor/identity_index.hpp>
#include <streamline/functional/functor/generic_stateless.hpp>

#include "sirius/vulkan/memory/resizable_gpu_buffer.hpp"



namespace acma::vk::impl {
	template<typename, auto, typename>
	struct buffer_tuple;
}

namespace acma::vk::impl {
	template<sl::size_t... Is, buffer_config_table<sizeof...(Is)> BufferConfigs, typename RenderProcessT>
	struct buffer_tuple<sl::index_sequence_type<Is...>, BufferConfigs, RenderProcessT> : sl::type_identity<
		sl::tuple<
			vk::resizable_gpu_buffer<
				std::next(BufferConfigs.begin(), Is)->value,
				RenderProcessT
			>...
		>
	> {};
}



namespace acma::vk::impl {
	template<typename Seq, auto BufferConfigs, typename RenderProcessT>
	using buffer_tuple_t = typename buffer_tuple<Seq, BufferConfigs, RenderProcessT>::type;
}
