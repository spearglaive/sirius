#pragma once

#include "sirius/vulkan/memory/asset_heap.hpp"

namespace acma::vk::impl {
	template<typename Seq, auto AssetHeapConfigs, typename RenderProcessT>
	struct asset_heap_group;
}


namespace acma::vk::impl {
	template<sl::index_t... Is, auto AssetHeapConfigs, typename RenderProcessT>
	struct asset_heap_group<sl::index_sequence_type<Is...>, AssetHeapConfigs, RenderProcessT> : 
		public asset_heap<
			sl::universal::get<sl::first_constant>(*std::next(AssetHeapConfigs.begin(), Is)),
			AssetHeapConfigs,
			RenderProcessT
		>... 
	{};
}