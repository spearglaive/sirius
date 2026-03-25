#pragma once

#include "sirius/vulkan/memory/asset_heap_allocation.hpp"

namespace acma::vk::impl {
	template<sl::size_t N, auto AssetHeapConfigs, typename RenderProcessT, typename Seq>
	struct asset_heap_allocation_group;
}


namespace acma::vk::impl {
	template<sl::size_t N, auto AssetHeapConfigs, typename RenderProcessT, sl::index_t... Is>
	struct asset_heap_allocation_group<N, AssetHeapConfigs, RenderProcessT, sl::index_sequence_type<Is...>> : 
		public asset_heap_allocation<
			N + Is, 
			sl::universal::get<sl::second_constant>(*std::next(AssetHeapConfigs.begin(), Is)),
			RenderProcessT
		>... 
	{
	protected:
		using asset_heap_allocation<
			N + Is, 
			sl::universal::get<sl::second_constant>(*std::next(AssetHeapConfigs.begin(), Is)),
			RenderProcessT
		>::realloc...;
	};
}


namespace acma::vk::impl {
	template<sl::size_t N, auto AssetHeapConfigs, typename RenderProcessT>
	struct asset_heap_allocation_group<N, AssetHeapConfigs, RenderProcessT, sl::index_sequence_type<>> {

		template<sl::index_t J>
		result<void> realloc(sl::index_constant_type<J>, sl::uint64_t = std::numeric_limits<sl::uint64_t>::max()) noexcept
		requires(J >= N) {
			return {};
		}
	};
}