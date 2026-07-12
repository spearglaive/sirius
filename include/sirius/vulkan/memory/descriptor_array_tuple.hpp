#pragma once
#include "sirius/core/render_process.fwd.hpp"

#include <streamline/functional/functor/subscript.hpp>
#include <streamline/functional/functor/identity_index.hpp>
#include <streamline/functional/functor/generic_stateless.hpp>

#include "sirius/core/descriptor_array_config_table.hpp"
#include "sirius/vulkan/memory/resizable_asset_descriptor_array.hpp"



namespace acma::vk::impl {
	template<typename, auto, typename>
	struct descriptor_array_tuple;
}

namespace acma::vk::impl {
	template<sl::size_t... Is, descriptor_array_config_table<sizeof...(Is)> DescriptorArrayConfigs, typename RenderProcessT>
	struct descriptor_array_tuple<sl::index_sequence_type<Is...>, DescriptorArrayConfigs, RenderProcessT> : sl::type_identity<
		sl::tuple<
			vk::resizable_asset_descriptor_array<
				std::next(DescriptorArrayConfigs.begin(), Is)->value,
				RenderProcessT
			>...
		>
	> {};
}



namespace acma::vk::impl {
	template<typename Seq, auto BufferConfigs, typename RenderProcessT>
	using descriptor_array_tuple_t = typename descriptor_array_tuple<Seq, BufferConfigs, RenderProcessT>::type;
}
