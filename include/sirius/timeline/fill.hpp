#pragma once 
#include <streamline/numeric/int.hpp>

#include "sirius/core/window.hpp"
#include "sirius/timeline/command.fwd.hpp"
#include "sirius/timeline/state.hpp"
#include "sirius/vulkan/core/command_buffer.hpp"
#include "sirius/core/buffer_config_table.hpp"
#include "sirius/core/asset_heap_config_table.hpp"
#include "sirius/timeline/event.hpp"


namespace acma {
	template<buffer_key_t BufferKey, sl::uint32_t Value = 0, sl::uoffset_t Offset = 0, sl::size_t CountBytes = VK_WHOLE_SIZE>
	struct fill : timeline::event {
		constexpr static command_family_t family = command_family::transfer;
	};
}

namespace acma::timeline {
	template<buffer_key_t BufferKey, sl::uint32_t Value, sl::uoffset_t Offset, sl::size_t CountBytes>
	struct command<fill<BufferKey, Value, Offset, CountBytes>> {
		template<typename RenderProcessT, sl::index_t CommandGroupIdx>
		constexpr result<void> operator()(RenderProcessT& proc, window&, timeline::state&, sl::empty_t, sl::index_constant_type<CommandGroupIdx>) const noexcept {
			vk::command_buffer const& cmd_buff = proc.command_buffers()[proc.frame_index()][CommandGroupIdx];
			cmd_buff.fill(proc[buffer_key_constant<BufferKey>], Value, Offset, CountBytes);
        	return {};
		};
	};
}