#pragma once 
#include <streamline/numeric/int.hpp>

#include "sirius/core/window.hpp"
#include "sirius/timeline/command.fwd.hpp"
#include "sirius/timeline/copy_buffer_data.hpp"
#include "sirius/timeline/state.hpp"
#include "sirius/vulkan/core/command_buffer.hpp"
#include "sirius/core/buffer_config_table.hpp"
#include "sirius/core/asset_heap_config_table.hpp"
#include "sirius/timeline/event.hpp"


namespace acma {
	template<buffer_key_t DstBufferKey, buffer_key_t SrcBufferKey>
	struct move_buffer_data : timeline::event {
		constexpr static command_family_t family = command_family::transfer;
	};
}

namespace acma::timeline {
	template<buffer_key_t DstBufferKey, buffer_key_t SrcBufferKey>
	struct command<move_buffer_data<DstBufferKey, SrcBufferKey>> {
		template<typename RenderProcessT, sl::index_t CommandGroupIdx>
		constexpr result<void> operator()(RenderProcessT& proc, window& _1, timeline::state& _2, sl::empty_t _3, sl::index_constant_type<CommandGroupIdx> _4) const noexcept {
			RESULT_VERIFY(sl::invoke(command<copy_buffer_data<DstBufferKey, SrcBufferKey>>{}, proc, _1, _2, _3, _4));
			auto& src_buff = sl::universal::get<SrcBufferKey>(proc);
			src_buff.clear();
			return {};
		};
	};
}