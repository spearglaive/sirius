#pragma once

#include "sirius/core/buffer_key_t.hpp"


namespace acma::vk {
	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
    class buffer;
}