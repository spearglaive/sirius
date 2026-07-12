#pragma once
#include <streamline/numeric/int.hpp>

#include "sirius/core/window.hpp"
#include "sirius/timeline/command.fwd.hpp"
#include "sirius/timeline/remake.hpp"
#include "sirius/timeline/state.hpp"
#include "sirius/vulkan/core/command_buffer.hpp"
#include "sirius/core/buffer_config_table.hpp"
#include "sirius/core/descriptor_array_config_table.hpp"
#include "sirius/timeline/event.hpp"


namespace acma {
	template<buffer_key_t DstBufferKey, sl::size_t SizeBytes, bool PreserveOldAllocation = true>
	struct recreate : timeline::event {
		constexpr static command_family_t family = command_family::transfer;
	};

	template<buffer_key_t DstBufferKey, buffer_key_t SrcBufferKey, bool PreserveOldAllocation = true>
	struct recreate_to_fit : timeline::event {
		constexpr static command_family_t family = command_family::transfer;
	};
}


namespace acma::timeline {
	template<buffer_key_t DstBufferKey, sl::size_t SizeBytes, bool PreserveOldAllocation>
	struct command<recreate<DstBufferKey, SizeBytes, PreserveOldAllocation>> {
		template<typename RenderProcessT, sl::index_t CommandGroupIdx, sl::size_t UserByteCount>
		constexpr result<void> operator()(RenderProcessT& proc, window&, timeline::state<UserByteCount>&, sl::empty_t, sl::index_constant_type<CommandGroupIdx>) const noexcept {
			RESULT_VERIFY((impl::remake<DstBufferKey, PreserveOldAllocation, false>(proc, sl::index_constant<CommandGroupIdx>, SizeBytes)));
			return {};
		};
	};


	template<buffer_key_t DstBufferKey, buffer_key_t SrcBufferKey, bool PreserveOldAllocation>
	struct command<recreate_to_fit<DstBufferKey, SrcBufferKey, PreserveOldAllocation>> {
		template<typename RenderProcessT, sl::index_t CommandGroupIdx, sl::size_t UserByteCount>
		constexpr result<void> operator()(RenderProcessT& proc, window&, timeline::state<UserByteCount>&, sl::empty_t, sl::index_constant_type<CommandGroupIdx>) const noexcept {
			auto const& src_buff = sl::universal::get<SrcBufferKey>(proc);
			RESULT_VERIFY((impl::remake<DstBufferKey, PreserveOldAllocation, false>(proc, sl::index_constant<CommandGroupIdx>, src_buff.size_bytes())));
			return {};
		};
	};
}
