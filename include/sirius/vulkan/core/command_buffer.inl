#pragma once
#include "sirius/core/dispatchable.hpp"
#include "sirius/core/drawable.hpp"
#include "sirius/core/index_buffer_info.hpp"
#include "sirius/vulkan/core/command_buffer.hpp"
#include <streamline/functional/functor/invoke_each.hpp>

#include "sirius/vulkan/core/vulkan.hpp"

#include "sirius/core/render_process.hpp"


namespace acma::vk {
	result<void> command_buffer::begin(bool one_time) const noexcept {
		VkCommandBufferBeginInfo begin_info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = one_time ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0U,
		};
		__D2D_VULKAN_VERIFY(sl::invoke(vulkan_fns_ptr->vkBeginCommandBuffer, smart_handle.get(), &begin_info));
		return {};
	}

	result<void> command_buffer::end() const noexcept {
		__D2D_VULKAN_VERIFY(sl::invoke(vulkan_fns_ptr->vkEndCommandBuffer, smart_handle.get()));
		return {};
	}


	result<void> command_buffer::reset() const noexcept {
		__D2D_VULKAN_VERIFY(sl::invoke(vulkan_fns_ptr->vkResetCommandBuffer, smart_handle.get(), 0));
		return {};
	}

	result<void> command_buffer::submit(
		command_family_t family,
		std::span<const semaphore_submit_info> wait_semaphore_infos,
		std::span<const semaphore_submit_info> signal_semaphore_infos,
		VkFence out_fence,
		sl::uint32_t queue_idx
	) const noexcept {
		VkCommandBufferSubmitInfo cmd_buff_info {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.pNext = nullptr,
			.commandBuffer = smart_handle.get(),
			.deviceMask = 0,
		};
		VkSubmitInfo2 submit_info {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.pNext = nullptr,
			.flags = 0,
			.waitSemaphoreInfoCount = static_cast<std::uint32_t>(wait_semaphore_infos.size()),
			.pWaitSemaphoreInfos = wait_semaphore_infos.data(),
			.commandBufferInfoCount = 1,
			.pCommandBufferInfos = &cmd_buff_info,
			.signalSemaphoreInfoCount = static_cast<std::uint32_t>(signal_semaphore_infos.size()),
			.pSignalSemaphoreInfos = signal_semaphore_infos.data(),
		};
		auto const& queues = logi_device_ptr->queues[family];
		__D2D_VULKAN_VERIFY(sl::invoke(vulkan_fns_ptr->vkQueueSubmit2, queues[queue_idx % queues.size()], 1, &submit_info, out_fence));
		return {};
	}

	result<void> command_buffer::wait(command_family_t family, sl::uint32_t queue_idx) const noexcept {
		auto const& queues = logi_device_ptr->queues[family];
		__D2D_VULKAN_VERIFY(sl::invoke(vulkan_fns_ptr->vkQueueWaitIdle, queues[queue_idx % queues.size()]));
		return {};
	}

	void command_buffer::free() const noexcept {
		return sl::invoke(vulkan_fns_ptr->vkFreeCommandBuffers, *logi_device_ptr, *cmd_pool_ptr, 1, &smart_handle.get());
	}
}


namespace acma::vk {
	void command_buffer::pipeline_barrier(std::span<const VkMemoryBarrier2> global_barriers, std::span<const VkBufferMemoryBarrier2> buffer_barriers, std::span<const VkImageMemoryBarrier2> image_barriers) const noexcept {
		VkDependencyInfo barrier_info{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = nullptr,
			.dependencyFlags = 0,
			.memoryBarrierCount = static_cast<std::uint32_t>(global_barriers.size()),
			.pMemoryBarriers = global_barriers.data(),
			.bufferMemoryBarrierCount = static_cast<std::uint32_t>(buffer_barriers.size()),
			.pBufferMemoryBarriers = buffer_barriers.data(),
			.imageMemoryBarrierCount = static_cast<std::uint32_t>(image_barriers.size()),
			.pImageMemoryBarriers = image_barriers.data(),
		};
		sl::invoke(vulkan_fns_ptr->vkCmdPipelineBarrier2, smart_handle.get(), &barrier_info);
	}
}

namespace acma::vk {
	void command_buffer::begin_draw(
		std::span<const VkRenderingAttachmentInfo> color_attachments, 
		VkRenderingAttachmentInfo const& depth_attachment, 
		rect<std::uint32_t> render_area_bounds,
		rect<float> viewport_bounds,
		rect<std::uint32_t> scissor_bounds
	) const noexcept {
		//Set render pass
		VkRenderingInfo rendering_info{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.renderArea{
				.offset = static_cast<VkOffset2D>(render_area_bounds.pos),
				.extent = static_cast<VkExtent2D>(render_area_bounds.size),
			},
			.layerCount = 1,
			.colorAttachmentCount = static_cast<std::uint32_t>(color_attachments.size()),
			.pColorAttachments = color_attachments.data(),
			.pDepthAttachment = &depth_attachment
		};
		sl::invoke(vulkan_fns_ptr->vkCmdBeginRendering, smart_handle.get(), &rendering_info);

		//Set viewport
		VkViewport v{viewport_bounds.x(), viewport_bounds.y(), viewport_bounds.width(), viewport_bounds.height(), 0.f, 1.f};
		sl::invoke(vulkan_fns_ptr->vkCmdSetViewport, smart_handle.get(), 0, 1, &v);

		//Set viewport crop
		VkRect2D scissor = static_cast<VkRect2D>(scissor_bounds);
		sl::invoke(vulkan_fns_ptr->vkCmdSetScissor, smart_handle.get(), 0, 1, &scissor);
	}


	void command_buffer::end_draw() const noexcept {
		sl::invoke(vulkan_fns_ptr->vkCmdEndRendering, smart_handle.get());
	}
}


namespace acma::vk {
	template<typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
	void command_buffer::bind_index_buffer(
		render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& render_proc
	) const noexcept requires has_index_info<T> {
		static_assert(T::pipeline_bind_point == bind_point::graphics);

		constexpr index_buffer_info info = T::index_info;

		constexpr VkIndexType index_type = [](){
			if constexpr(info.index_size == sizeof(sl::uint32_t))
				return VK_INDEX_TYPE_UINT32;
			return VK_INDEX_TYPE_UINT16;
		}();

		sl::invoke(vulkan_fns_ptr->vkCmdBindIndexBuffer,
			smart_handle.get(),
			sl::universal::get<info.buffer_key>(render_proc).handle(),
			info.offset,
			index_type
		);
	}
	

	template<bind_point_t BindPoint, typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
	void command_buffer::bind_push_constants(
		render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& render_proc,
		pipeline_layout<BindPoint, T, BufferConfigs, AssetHeapConfigs> const& layout
	) const noexcept requires has_push_constants<T> {
		constexpr auto bind_push_constant = []<sl::index_t I>(
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& proc,
			pipeline_layout<BindPoint, T, BufferConfigs, AssetHeapConfigs> const& p_layout,
			command_buffer const& cmd_buff,
			sl::index_constant_type<I>
		) noexcept {
			constexpr push_constant_buffer_info info = T::push_constant_infos[I];
			constexpr buffer_config config = BufferConfigs[info.buffer_key];
			sl::invoke(
				cmd_buff.vulkan_fns_ptr->vkCmdPushConstants,
				cmd_buff,
				p_layout,
				config.stages,
				info.offset,
				config.initial_capacity_bytes,
				sl::universal::get<info.buffer_key>(proc).data()
			);
		};
		
		sl::invoke(
			sl::functor::invoke_each<bind_push_constant>{},
			sl::index_sequence_of_length<T::push_constant_infos.size()>,
			render_proc,
			layout,
			*this
		);
	}

	
	template<bind_point_t BindPoint, typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
	void command_buffer::bind_uniform_buffers(
		render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& render_proc,
		pipeline_layout<BindPoint, T, BufferConfigs, AssetHeapConfigs> const& layout
	) const noexcept requires has_uniform_buffers<T> {
		const auto buffer_infos = sl::make<sl::array<T::uniform_buffers.size(), VkDescriptorBufferInfo>>(
			render_proc,
			sl::in_place_repeat_tag<T::uniform_buffers.size()>,
			[]<sl::index_t I>(
				render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& proc,
				sl::index_constant_type<I>
			) noexcept {
				auto const& buff = sl::universal::get<T::uniform_buffers[I]>(proc);
				return VkDescriptorBufferInfo{
					.buffer = buff.handle(),
					.offset = 0,
					.range = buff.size_bytes(),
				};
			}
		);

		const auto writes = sl::make<sl::array<T::uniform_buffers.size(), VkWriteDescriptorSet>>(
			buffer_infos,
			[]<sl::index_t I>(VkDescriptorBufferInfo const& info, sl::index_constant_type<I>) noexcept {
				return VkWriteDescriptorSet{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.pNext = nullptr,
					.dstBinding = I,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.pBufferInfo = &info
				};
			}
		);
		
		sl::invoke(vulkan_fns_ptr->vkCmdPushDescriptorSetKHR,
			smart_handle.get(),
			static_cast<VkPipelineBindPoint>(BindPoint),
			layout,
			0, 
			writes.size(),
			writes.data()
		);
	}

	template<bind_point_t BindPoint, typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
	void command_buffer::bind_asset_heap(
		render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& render_proc,
		pipeline_layout<BindPoint, T, BufferConfigs, AssetHeapConfigs> const& layout
	) const noexcept requires has_asset_heap<T> {
		const sl::array<asset_usage_policy::num_usage_policies, VkDescriptorSet> descriptor_set_handles = 
			sl::universal::make_deduced<sl::generic::array>(sl::universal::get<T::asset_heap>(render_proc).descriptor_sets(), sl::functor::forward_construct<VkDescriptorSet>{});

		sl::invoke(vulkan_fns_ptr->vkCmdBindDescriptorSets,
			smart_handle.get(),
			static_cast<VkPipelineBindPoint>(BindPoint),
			layout,
			1,
			asset_usage_policy::num_usage_policies,
			descriptor_set_handles.data(),
			//sl::universal::get<T::asset_heap>(render_proc).descriptor_set_handles().data(), 
			0, nullptr
		);
	}


	template<bind_point_t BindPoint, typename T, auto BufferConfigs, auto AssetHeapConfigs>
	void command_buffer::bind_pipeline(pipeline<BindPoint, T, BufferConfigs, AssetHeapConfigs> const& p) const noexcept {
		sl::invoke(vulkan_fns_ptr->vkCmdBindPipeline, smart_handle.get(), static_cast<VkPipelineBindPoint>(BindPoint), p);
	}
}


namespace acma::vk {
	template<typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
	void command_buffer::draw(
		render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& render_proc
	) const noexcept {
		constexpr static auto draw_cmd_buff_offsets = sl::make<sl::array<decltype(T::draw_infos)::size(), sl::uoffset_t>>(
			T::draw_infos,
			[]<sl::index_t I>(draw_info info, sl::index_constant_type<I>) noexcept {
				return info.draw_command_offset;
			}
		);

		constexpr static auto draw_cnt_buff_offsets = sl::make<sl::array<decltype(T::draw_infos)::size(), sl::uoffset_t>>(
			T::draw_infos,
			[]<sl::index_t I>(draw_info info, sl::index_constant_type<I>) noexcept {
				return info.draw_count_offset;
			}
		);

		return draw<T>(
			render_proc,
			draw_cmd_buff_offsets,
			draw_cnt_buff_offsets
		);
	}

	
	template<typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
	void command_buffer::dispatch(
		render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& render_proc
	) const noexcept {
		constexpr static auto offsets = sl::make<sl::array<decltype(T::dispatch_infos)::size(), sl::uoffset_t>>(
			T::dispatch_infos,
			[]<sl::index_t I>(dispatch_info info, sl::index_constant_type<I>) noexcept {
				return info.offset;
			}
		);

		return dispatch<T>(
			render_proc,
			offsets
		);
	}
}

namespace acma::vk {
	template<typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
	void command_buffer::draw(
		render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& render_proc,
		sl::array<decltype(T::draw_infos)::size(), sl::uoffset_t> draw_command_buffer_offsets, 
		sl::array<decltype(T::draw_infos)::size(), sl::uoffset_t> draw_count_buffer_offsets 
	) const noexcept {
		constexpr auto draw_command = []<sl::index_t I>(
			VkCommandBuffer cmd_buff,
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& proc,
			sl::array<decltype(T::draw_infos)::size(), sl::uoffset_t> draw_cmd_buff_offsets, 
			sl::array<decltype(T::draw_infos)::size(), sl::uoffset_t> draw_cnt_buff_offsets,
			sl::index_constant_type<I>
		) noexcept -> void {
			const sl::uoffset_t draw_cmd_offset = draw_cmd_buff_offsets[I];
			const sl::uoffset_t draw_cnt_offset = draw_cnt_buff_offsets[I];
			constexpr draw_info info = T::draw_infos[I];
			auto const& draw_cmd_buff = sl::universal::get<info.draw_command_buffer_key>(proc);
			auto const& draw_cnt_buff = sl::universal::get<info.draw_count_buffer_key>(proc);

			if constexpr (has_index_info<T>) {
				constexpr static sl::size_t stride = sizeof(indexed_draw_command_t);
				const sl::uint32_t final_max_draw_count = std::min(T::max_draw_count(), static_cast<sl::uint32_t>((draw_cmd_buff.capacity_bytes() - stride - draw_cmd_offset)/stride) + 1);
				sl::invoke(proc.vulkan_functions_ptr()->vkCmdDrawIndexedIndirectCount, cmd_buff, static_cast<VkBuffer>(draw_cmd_buff), draw_cmd_offset, static_cast<VkBuffer>(draw_cnt_buff), draw_cnt_offset, final_max_draw_count, stride);
			} else {
				constexpr static sl::size_t stride = sizeof(draw_command_t);
				const sl::uint32_t final_max_draw_count = std::min(T::max_draw_count(), static_cast<sl::uint32_t>((draw_cmd_buff.capacity_bytes() - stride - draw_cmd_offset)/stride) + 1);
				sl::invoke(proc.vulkan_functions_ptr()->vkCmdDrawIndirectCount, cmd_buff, static_cast<VkBuffer>(draw_cmd_buff), draw_cmd_offset, static_cast<VkBuffer>(draw_cnt_buff), draw_cnt_offset, final_max_draw_count, stride);
			}
		};

		return sl::functor::invoke_each<draw_command>{}(sl::index_sequence_of_length<decltype(T::draw_infos)::size()>, this->smart_handle.get(), render_proc, draw_command_buffer_offsets, draw_count_buffer_offsets);
	}
	
	template<typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
	void command_buffer::dispatch(
		render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& render_proc,
		sl::array<decltype(T::dispatch_infos)::size(), sl::uoffset_t> buffer_offsets
	) const noexcept {
		constexpr auto dispatch_command = []<sl::index_t I>(
			VkCommandBuffer cmd_buff,
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& proc,
			sl::array<decltype(T::dispatch_infos)::size(), sl::uoffset_t> buff_offsets, 
			sl::index_constant_type<I>
		) noexcept -> void {
			constexpr dispatch_info info = T::dispatch_infos[I];
			sl::invoke(proc.vulkan_functions_ptr()->vkCmdDispatchIndirect, cmd_buff, static_cast<VkBuffer>(sl::universal::get<info.buffer_key>(proc)), buff_offsets[I]);
		};

		return sl::functor::invoke_each<dispatch_command>{}(sl::index_sequence_of_length<decltype(T::dispatch_infos)::size()>, this->smart_handle.get(), render_proc, buffer_offsets);
	}
}


namespace acma::vk {
	void command_buffer::copy(
		vk::buffer_allocation_unique_ptr& dst, 
		vk::buffer_allocation_unique_ptr const& src, 
		std::span<const VkBufferCopy> copy_regions
	) const noexcept {
		return sl::invoke(vulkan_fns_ptr->vkCmdCopyBuffer, smart_handle.get(), src->handle, dst->handle, copy_regions.size(), copy_regions.data());
	}


	void command_buffer::copy(
		vk::buffer_allocation_unique_ptr& dst, 
		vk::buffer_allocation_unique_ptr const& src, 
		std::size_t size, 
		sl::uoffset_t dst_offset, 
		sl::uoffset_t src_offset
	) const noexcept {
		VkBufferCopy copy_region{ 
			.srcOffset = src_offset,
			.dstOffset = dst_offset,
			.size = size,
		};
		return copy(dst, src, {&copy_region, 1});
	}
}

namespace acma::vk {
	template<buffer_key_t DstK, buffer_key_t SrcK, auto BufferConfigs, typename RenderProcessT>
	void command_buffer::copy(
		buffer<DstK, BufferConfigs, RenderProcessT>& dst, 
		buffer<SrcK, BufferConfigs, RenderProcessT> const& src, 
		std::span<const VkBufferCopy> copy_regions
	) const noexcept {
		return copy(dst.allocation_ptr(), src.allocation_ptr(), copy_regions);
	}


	template<buffer_key_t DstK, buffer_key_t SrcK, auto BufferConfigs, typename RenderProcessT>
	void command_buffer::copy(
		buffer<DstK, BufferConfigs, RenderProcessT>& dst, 
		buffer<SrcK, BufferConfigs, RenderProcessT> const& src, 
		std::size_t size, 
		sl::uoffset_t dst_offset, 
		sl::uoffset_t src_offset
	) const noexcept {
		VkBufferCopy copy_region{ 
			.srcOffset = src_offset,
			.dstOffset = dst_offset,
			.size = size,
		};
		return copy(dst, src, {&copy_region, 1});
	}
}

namespace acma::vk {
	template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
	void command_buffer::fill(
		buffer<K, BufferConfigs, RenderProcessT>& dst,
		sl::uint32_t value,
		sl::uoffset_t dst_offset,
		sl::size_t fill_count_bytes
	) const noexcept {
		sl::invoke(vulkan_fns_ptr->vkCmdFillBuffer, smart_handle.get(), dst.handle(), dst_offset, fill_count_bytes, value);
	}
}



namespace acma::impl {
	result<vk::command_buffer>
		make<vk::command_buffer>::
	operator()(
		sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
		sl::reference_ptr<const vk::physical_device> phys_device_ptr,
		sl::reference_ptr<const vk::logical_device> logi_device_ptr,
		sl::reference_ptr<const vk::command_pool> cmd_pool_ptr,
		sl::in_place_adl_tag_type<vk::command_buffer>
	) const noexcept {
		vk::command_buffer ret{};
		ret.smart_handle = {vulkan_fns_ptr->vkFreeCommandBuffers, logi_device_ptr, cmd_pool_ptr};
		ret.vulkan_fns_ptr = vulkan_fns_ptr;
		ret.logi_device_ptr = logi_device_ptr;
		ret.phys_device_ptr = phys_device_ptr;
		ret.cmd_pool_ptr = cmd_pool_ptr;

		const VkCommandBufferAllocateInfo alloc_info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = *cmd_pool_ptr,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};

		__D2D_VULKAN_VERIFY(sl::invoke(vulkan_fns_ptr->vkAllocateCommandBuffers, *logi_device_ptr, &alloc_info, &ret.smart_handle.get()));
		return ret;
	}
}