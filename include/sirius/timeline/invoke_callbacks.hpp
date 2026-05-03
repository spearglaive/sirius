#pragma once
#include <streamline/numeric/int.hpp>

#include "sirius/core/window.hpp"
#include "sirius/core/invoke_all.def.hpp"
#include "sirius/timeline/callback_event.hpp"
#include "sirius/timeline/command.fwd.hpp"
#include "sirius/timeline/remake.hpp"
#include "sirius/timeline/state.hpp"
#include "sirius/vulkan/core/command_buffer.hpp"
#include "sirius/core/buffer_config_table.hpp"
#include "sirius/core/asset_heap_config_table.hpp"
#include "sirius/timeline/event.hpp"


namespace acma {
	template<sl::index_t EventId>
	struct invoke_callbacks : timeline::event {
		constexpr static command_family_t family = command_family::transfer;
	};
}

namespace acma::timeline {
	template<sl::index_t EventId>
	struct command<invoke_callbacks<EventId>> {
		template<typename RenderProcessT, sl::index_t CommandGroupIdx, sl::size_t UserByteCount>
		constexpr result<void> operator()(
			RenderProcessT& proc,
			window& win,
			timeline::state<UserByteCount>& timeline_state,
			sl::empty_t,
			sl::index_constant_type<CommandGroupIdx>
		) const noexcept {
			for(sl::index_t i = 0; i < proc.timeline_callbacks()[EventId].size(); ++i)
				RESULT_VERIFY(sl::invoke(proc.timeline_callbacks()[EventId][i], proc, win, timeline_state));
			return {};
		};
	};
}
