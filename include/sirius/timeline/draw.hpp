#pragma once
#include <streamline/metaprogramming/integer_sequence.hpp>

#include "sirius/timeline/command.fwd.hpp"
#include "sirius/timeline/setup.hpp"
#include "sirius/core/window.hpp"
#include "sirius/core/render_process.hpp"
#include "sirius/vulkan/memory/pipeline.hpp"
#include "sirius/timeline/state.hpp"
#include "sirius/timeline/event.hpp"


namespace acma {
	template<typename T>
	struct draw : timeline::event {
		constexpr static command_family_t family = command_family::graphics;
	};
}


namespace acma::timeline {
	template<typename T>
	struct setup<draw<T>> {
		template<auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
		result<vk::pipeline<vk::bind_point::graphics, T, BufferConfigs, AssetHeapConfigs>> operator()(
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& proc,
			window& win
		) const noexcept {
			return make<vk::pipeline<vk::bind_point::graphics, T, BufferConfigs, AssetHeapConfigs>>(
				proc.vulkan_functions_ptr(),
				proc.logical_device_ptr(),
				proc,
				std::span<const VkFormat, 1>{&win.swap_chain_ptr()->format().pixel_format.id, 1}, 
				win.depth_image_ptr()->format_id()
			);
		};
	};
}

namespace acma::timeline {
	template<typename T>
	struct command<draw<T>> {
		template<auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount, sl::index_t CommandGroupIdx>
		constexpr result<void> operator()(
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& proc, 
			window&, 
			timeline::state&, 
			vk::pipeline<vk::bind_point::graphics, T, BufferConfigs, AssetHeapConfigs>& pipeline, 
			sl::index_constant_type<CommandGroupIdx>
		) const noexcept {
			vk::command_buffer const& graphics_buffer = proc.command_buffers()[proc.frame_index()][CommandGroupIdx];

			graphics_buffer.bind_pipeline(pipeline);

			graphics_buffer.template bind_index_buffer<T>(proc);
			graphics_buffer.bind_push_constants(proc, pipeline.layout());
			graphics_buffer.bind_uniform_buffers(proc, pipeline.layout());
			graphics_buffer.bind_asset_heap(proc, pipeline.layout());

			graphics_buffer.template draw<T>(proc); 

			return {};
		};
	};
}