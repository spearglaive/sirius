#pragma once

#include "sirius/core/asset_heap_key_t.hpp"


namespace acma::vk {
	template<asset_heap_key_t K, auto AssetHeapConfigs, typename RenderProcessT>
	class asset_heap;
}
