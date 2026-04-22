#pragma once
#include "sirius/vulkan/core/command_buffer.fwd.hpp"

#include <cstddef>
#include <memory>
#include <span>
#include <streamline/memory/reference_ptr.hpp>

#include <frozen/unordered_map.h>

#include "sirius/vulkan/core/vulkan.hpp"
#include "sirius/core/buffer_config_table.hpp"
#include "sirius/core/render_process.fwd.hpp"
#include "sirius/vulkan/memory/bind_point.hpp"
#include "sirius/vulkan/memory/buffer.hpp"
#include "sirius/vulkan/memory/asset_heap.hpp"
#include "sirius/vulkan/memory/image.hpp"
#include "sirius/vulkan/device/logical_device.hpp"
#include "sirius/vulkan/core/command_pool.hpp"
#include "sirius/arith/rect.hpp"
#include "sirius/vulkan/memory/pipeline.hpp"
#include "sirius/vulkan/sync/semaphore.hpp"

namespace acma::vk {
    struct command_buffer : mixin<VkCommandBuffer, PFN_vkFreeCommandBuffers, logical_device, command_pool> {
	public:
		template<typename T>
		friend struct ::acma::impl::make;

	private:
		template<typename T> constexpr static bool has_index_info = requires{ T::index_info; };
		template<typename T> constexpr static bool has_asset_heap = requires{ T::asset_heap; };
		template<typename T> constexpr static bool has_push_constants = T::push_constant_infos.size() > 0;
		template<typename T> constexpr static bool has_uniform_buffers = T::uniform_buffers.size() > 0;


	public:
    	inline result<void> begin(bool one_time = false) const noexcept;
    	inline result<void> end() const noexcept;

    	inline result<void> reset() const noexcept;
    	inline result<void> submit(command_family_t family, std::span<const semaphore_submit_info> wait_semaphore_infos = {}, std::span<const semaphore_submit_info> signal_semaphore_infos = {}, VkFence out_fence = VK_NULL_HANDLE, sl::uint32_t queue_idx = 0) const noexcept;
    	inline result<void> wait(command_family_t family, sl::uint32_t queue_idx = 0) const noexcept;
    	inline void free() const noexcept;
		
	public:
		template<bind_point_t BindPoint, typename T, auto BufferConfigs, auto AssetHeapConfigs>
		void bind_pipeline(pipeline<BindPoint, T, BufferConfigs, AssetHeapConfigs> const& p) const noexcept;
	public:
		template<typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
		void bind_index_buffer(
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& render_proc
		) const noexcept requires has_index_info<T>;

		template<bind_point_t BindPoint, typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
		void bind_push_constants(
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& render_proc,
			pipeline_layout<BindPoint, T, BufferConfigs, AssetHeapConfigs> const& layout
		) const noexcept requires has_push_constants<T>;

		template<bind_point_t BindPoint, typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
		void bind_uniform_buffers(
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& render_proc,
			pipeline_layout<BindPoint, T, BufferConfigs, AssetHeapConfigs> const& layout
		) const noexcept requires has_uniform_buffers<T>;

		template<bind_point_t BindPoint, typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
		void bind_asset_heap(
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& render_proc,
			pipeline_layout<BindPoint, T, BufferConfigs, AssetHeapConfigs> const& layout
		) const noexcept requires has_asset_heap<T>;
	public:
		template<typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
		void bind_index_buffer(
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const&
		) const noexcept requires (!has_index_info<T>) {}

		template<bind_point_t BindPoint, typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
		void bind_push_constants(
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const&,
			pipeline_layout<BindPoint, T, BufferConfigs, AssetHeapConfigs> const&
		) const noexcept requires (!has_push_constants<T>) {}

		template<bind_point_t BindPoint, typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
		void bind_uniform_buffers(
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const&,
			pipeline_layout<BindPoint, T, BufferConfigs, AssetHeapConfigs> const&
		) const noexcept requires (!has_uniform_buffers<T>) {}

		template<bind_point_t BindPoint, typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
		void bind_asset_heap(
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const&,
			pipeline_layout<BindPoint, T, BufferConfigs, AssetHeapConfigs> const&
		) const noexcept requires (!has_asset_heap<T>) {}

	public:
        inline void begin_draw(std::span<const VkRenderingAttachmentInfo> color_attachments, VkRenderingAttachmentInfo const& depth_attachment, rect<std::uint32_t> render_area_bounds, rect<float> viewport_bounds, rect<std::uint32_t> scissor_bounds) const noexcept;
        inline void end_draw() const noexcept;

    public:
		template<typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
        void draw(
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& render_proc
		) const noexcept;

		template<typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
        void dispatch(
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& render_proc
		) const noexcept;

    public:
		template<typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
        void draw(
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& render_proc,
			sl::array<decltype(T::draw_infos)::size(), sl::uoffset_t> draw_command_buffer_offsets, 
			sl::array<decltype(T::draw_infos)::size(), sl::uoffset_t> draw_count_buffer_offsets
		) const noexcept;

		template<typename T, auto BufferConfigs, auto AssetHeapConfigs, sl::size_t CommandGroupCount>
        void dispatch(
			render_process<BufferConfigs, AssetHeapConfigs, CommandGroupCount> const& render_proc,
			sl::array<decltype(T::dispatch_infos)::size(), sl::uoffset_t> buffer_offsets
		) const noexcept;
        
    public:
        inline void copy(vk::buffer_allocation_unique_ptr& dst, vk::buffer_allocation_unique_ptr const& src, std::span<const VkBufferCopy> copy_regions) const noexcept;
        inline void copy(vk::buffer_allocation_unique_ptr& dst, vk::buffer_allocation_unique_ptr const& src, std::size_t size, sl::uoffset_t dst_offset = 0, sl::uoffset_t src_offset = 0) const noexcept;

		template<buffer_key_t DstK, buffer_key_t SrcK, auto BufferConfigs, typename RenderProcessT>
        void copy(buffer<DstK, BufferConfigs, RenderProcessT>& dst, buffer<SrcK, BufferConfigs, RenderProcessT> const& src, std::span<const VkBufferCopy> copy_regions) const noexcept;
		template<buffer_key_t DstK, buffer_key_t SrcK, auto BufferConfigs, typename RenderProcessT>
        void copy(buffer<DstK, BufferConfigs, RenderProcessT>& dst, buffer<SrcK, BufferConfigs, RenderProcessT> const& src, std::size_t size, sl::uoffset_t dst_offset = 0, sl::uoffset_t src_offset = 0) const noexcept;

		template<buffer_key_t K, auto BufferConfigs, typename RenderProcessT>
        void fill(buffer<K, BufferConfigs, RenderProcessT>& dst, sl::uint32_t value = 0, sl::uoffset_t dst_offset = 0, sl::size_t fill_count_bytes = VK_WHOLE_SIZE) const noexcept;

    public:
        inline void pipeline_barrier(std::span<const VkMemoryBarrier2> global_barriers, std::span<const VkBufferMemoryBarrier2> buffer_barriers, std::span<const VkImageMemoryBarrier2> image_barriers) const noexcept;
    
	private:
        constexpr static frozen::unordered_map<VkImageLayout, std::pair<VkPipelineStageFlagBits2, VkAccessFlagBits2>, 4> image_barrier_map {
            {VK_IMAGE_LAYOUT_UNDEFINED,                {VK_PIPELINE_STAGE_2_NONE,                VK_ACCESS_2_NONE}},
            {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, {VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT}},
            {VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,     {VK_PIPELINE_STAGE_2_TRANSFER_BIT,        VK_ACCESS_2_TRANSFER_READ_BIT}},
            {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,     {VK_PIPELINE_STAGE_2_TRANSFER_BIT,        VK_ACCESS_2_TRANSFER_WRITE_BIT}},
        };

    private:
		sl::reference_ptr<const vk::function_table> vulkan_fns_ptr;
        sl::reference_ptr<const physical_device> phys_device_ptr;
        sl::reference_ptr<const logical_device> logi_device_ptr;
        sl::reference_ptr<const command_pool> cmd_pool_ptr;
    };
}


namespace acma::impl {
	template<>
    struct make<vk::command_buffer> {
		SIRIUS_API inline result<vk::command_buffer> operator()(
			sl::reference_ptr<const vk::function_table> vulkan_fns_ptr,
			sl::reference_ptr<const vk::physical_device> phys_device_ptr,
			sl::reference_ptr<const vk::logical_device> logi_device_ptr,
			sl::reference_ptr<const vk::command_pool> cmd_pool_ptr,
			sl::in_place_adl_tag_type<vk::command_buffer> = sl::in_place_adl_tag<vk::command_buffer>
		) const noexcept;
	};
}

#include "sirius/vulkan/core/command_buffer.inl"
