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
	template<buffer_key_t DstBufferKey, sl::size_t SizeBytes>
	struct resize : timeline::event {
		constexpr static command_family_t family = command_family::transfer;
	};

	template<buffer_key_t DstBufferKey, buffer_key_t SrcBufferKey>
	struct resize_to_fit : timeline::event {
		constexpr static command_family_t family = command_family::transfer;
	};
}

namespace acma::timeline {
	template<buffer_key_t DstBufferKey, sl::size_t SizeBytes>
	struct command<resize<DstBufferKey, SizeBytes>> {
		template<typename RenderProcessT, sl::index_t CommandGroupIdx, sl::size_t UserByteCount>
		constexpr result<void> operator()(RenderProcessT& proc, window&, timeline::state<UserByteCount>&, sl::empty_t, sl::index_constant_type<CommandGroupIdx>) const noexcept {
			return impl::remake<DstBufferKey, true, true>(proc, sl::index_constant<CommandGroupIdx>, SizeBytes);
		};
	};


	template<buffer_key_t DstBufferKey, buffer_key_t SrcBufferKey>
	struct command<resize_to_fit<DstBufferKey, SrcBufferKey>> {
		template<typename RenderProcessT, sl::index_t CommandGroupIdx, sl::size_t UserByteCount>
		constexpr result<void> operator()(RenderProcessT& proc, window&, timeline::state<UserByteCount>&, sl::empty_t, sl::index_constant_type<CommandGroupIdx>) const noexcept {
			auto const& src_buff = sl::universal::get<SrcBufferKey>(proc);
			return impl::remake<DstBufferKey, true, true>(proc, sl::index_constant<CommandGroupIdx>, src_buff.size_bytes());
		};
	};
}
