#pragma once
#include "sirius/core/memory_management.hpp"

#include <algorithm>

#include "sirius/timeline/dedicated_command_group.hpp"
#include "sirius/vulkan/memory/asset_heap.hpp"
#include "sirius/vulkan/memory/buffer.hpp"
#include "sirius/vulkan/memory/image.hpp"
#include "sirius/vulkan/memory/texture_data_info.hpp"


namespace acma {
	template<buffer_config DstConfig, buffer_config SrcConfig, typename RenderProcessT>
	constexpr result<void> gpu_copy(
		RenderProcessT& process,
		vk::buffer_allocation_unique_ptr& dst,
		sl::constant_type<buffer_config, DstConfig>,
		vk::buffer_allocation_unique_ptr const& src,
		sl::constant_type<buffer_config, SrcConfig>,
		sl::size_t size,
		sl::uoffset_t dst_offset,
		sl::uoffset_t src_offset
	) noexcept {
		return process.buffer_copy(dst, sl::constant<buffer_config, DstConfig>, src, sl::constant<buffer_config, SrcConfig>, size, dst_offset, src_offset);
	}

	template<buffer_key_t DstK, buffer_key_t SrcK, auto BufferConfigs, typename RenderProcessT>
	constexpr result<void> gpu_copy(
		vk::buffer<DstK, BufferConfigs, RenderProcessT>& dst,
		vk::buffer<SrcK, BufferConfigs, RenderProcessT> const& src,
		sl::size_t size,
		sl::uoffset_t dst_offset,
		sl::uoffset_t src_offset
	) noexcept {
		return gpu_copy(
			static_cast<RenderProcessT&>(dst),
			dst.allocation_ptr(),
			sl::constant<buffer_config, BufferConfigs[DstK]>,
			src.allocation_ptr(), 
			sl::constant<buffer_config, BufferConfigs[SrcK]>,
			size, dst_offset, src_offset
		);
	}
}

namespace acma{
	template<buffer_key_t DstK, buffer_key_t SrcK, auto BufferConfigs, typename RenderProcessT>
	constexpr result<void> gpu_copy(
		RenderProcessT& proc,
		vk::image& dst,
		vk::image const& src,
		extent3 size,
		offset3 dst_offset,
		offset3 src_offset,
		sl::uint64_t timeout
	) noexcept {
		const sl::index_t frame_idx = proc.frame_index();
		auto const& transfer_command_buffer = proc.command_buffers()[frame_idx][timeline::impl::dedicated_command_group::out_of_timeline_copy];

		RESULT_TRY_COPY_UNSCOPED(const sl::uint64_t post_copy_wait_value, proc.begin_dedicated_copy(timeline::impl::dedicated_command_group::out_of_timeline_copy, timeout), pcwv_result);


		if(size.width() == 0 || size.height() == 0 || size.depth() == 0) return {};
		
		const VkImageLayout original_layout = src.layout();

		sl::array<2, VkImageMemoryBarrier2> pre_copy_barriers {{
			VkImageMemoryBarrier2{
			    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
				.srcAccessMask = VK_ACCESS_2_NONE,
				.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
				.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
			    .oldLayout = original_layout,
			    .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			    .image = *src.handle_ref(),
			    .subresourceRange = {
			        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			        .baseMipLevel = 0,
			        .levelCount = src.mip_level_count(),
			        .baseArrayLayer = 0,
			        .layerCount = src.layer_count(),
			    },
			},
			VkImageMemoryBarrier2{
			    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
				.srcAccessMask = VK_ACCESS_2_NONE,
				.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
				.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
			    .oldLayout = dst.layout(),
			    .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			    .image = *dst.handle_ref(),
			    .subresourceRange = {
			        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			        .baseMipLevel = 0,
			        .levelCount = dst.mip_level_count(),
			        .baseArrayLayer = 0,
			        .layerCount = dst.layer_count(),
			    },
			},
		}};
		transfer_command_buffer.pipeline_barrier({}, {}, pre_copy_barriers);

		std::unique_ptr<VkImageCopy[]> copy_regions = std::make_unique_for_overwrite<VkImageCopy[]>(src.mip_level_count());
		for(sl::uint32_t i = 0; i < src.mip_level_count(); ++i) {
			new (&copy_regions[i]) VkImageCopy{
				VkImageSubresourceLayers{
				    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				    .mipLevel = i,
				    .baseArrayLayer = 0,
				    .layerCount = src.layer_count(),
				},
				std::bit_cast<VkOffset3D>(src_offset),
				VkImageSubresourceLayers{
				    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				    .mipLevel = i,
				    .baseArrayLayer = 0,
				    .layerCount = dst.layer_count(),
				},
				std::bit_cast<VkOffset3D>(dst_offset),
				std::bit_cast<VkExtent3D>(size)
			};
		}

		sl::invoke(proc.vulkan_functions_ptr()->vkCmdCopyImage,
			transfer_command_buffer, 
			src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			src.mip_level_count(), copy_regions.get()
		);

		if(original_layout == VK_IMAGE_LAYOUT_UNDEFINED) 
			dst.current_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; \
		else {
			sl::array<1, VkImageMemoryBarrier2> post_copy_barriers{{
				VkImageMemoryBarrier2{
				    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
					.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
					.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
					.dstStageMask = VK_PIPELINE_STAGE_2_NONE,
					.dstAccessMask = VK_ACCESS_2_NONE,
				    .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				    .newLayout = original_layout,
				    .image = *dst.handle_ref(),
				    .subresourceRange = {
				        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				        .baseMipLevel = 0,
				        .levelCount = dst.mip_level_count(),
				        .baseArrayLayer = 0,
				        .layerCount = dst.layer_count(),
				    },
				},
			}};
			transfer_command_buffer.pipeline_barrier({}, {}, post_copy_barriers);

			dst.current_layout = original_layout;
		}		

		return proc.end_dedicated_copy(post_copy_wait_value, timeline::impl::dedicated_command_group::out_of_timeline_copy, timeout);
	}
}
	
namespace acma{
	template<asset_heap_key_t DstK, buffer_key_t SrcK, auto AssetHeapConfigs, auto BufferConfigs, typename RenderProcessT>
	constexpr result<void> gpu_copy(
		vk::asset_heap<DstK, AssetHeapConfigs, RenderProcessT>& dst,
		vk::buffer<SrcK, BufferConfigs, RenderProcessT> const& src,
		sl::uint64_t timeout
	) noexcept {
		static_assert(
			(vk::buffer<SrcK, BufferConfigs, RenderProcessT>::config.usage & buffer_usage_policy::texture_data) == buffer_usage_policy::texture_data,
			"Source buffer in buffer -> asset heap copy must have `buffer_usage_policy::texture_data` as its usage policy"
		);
		
		RenderProcessT& proc = static_cast<RenderProcessT&>(dst);

		const sl::index_t frame_idx = proc.frame_index();
		const sl::index_t alloc_idx = dst.allocation_index();
		auto const& transfer_command_buffer = proc.command_buffers()[frame_idx][timeline::impl::dedicated_command_group::out_of_timeline_copy];

		RESULT_TRY_COPY_UNSCOPED(const sl::uint64_t post_copy_wait_value, proc.begin_dedicated_copy(timeline::impl::dedicated_command_group::out_of_timeline_copy, timeout), pcwv_result);
		
		std::vector<vk::texture_data_info> const& texture_data_infos = src.texture_data_infos;
		for(sl::index_t i = 0; i < texture_data_infos.size(); ++i) {
			if(texture_data_infos[i].size == 0) continue;
			
			vk::image& dst_image = dst._images[alloc_idx][dst.initialized_image_counts[alloc_idx] + i];

			//VkBufferMemoryBarrier2 pre_copy_buffer_barrier{
			//    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			//	.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
			//	.srcAccessMask = VK_ACCESS_2_NONE,
			//	.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
			//	.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
			//	.buffer = static_cast<VkBuffer>(texture_data_buffer),
			//	.offset = texture_data_infos[i].offset,
			//	.size = texture_data_infos[i].size
			//};
			VkImageMemoryBarrier2 pre_copy_image_barrier{
			    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
				.srcAccessMask = VK_ACCESS_2_NONE,
				.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
				.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
			    .oldLayout = dst_image.layout(),
			    .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			    .image = *dst_image.handle_ref(),
			    .subresourceRange = {
			        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			        .baseMipLevel = 0,
			        .levelCount = dst_image.mip_level_count(),
			        .baseArrayLayer = 0,
			        .layerCount = dst_image.layer_count(),
			    },
			};
			transfer_command_buffer.pipeline_barrier({}, {/*&pre_copy_buffer_barrier, 1*/}, {&pre_copy_image_barrier, 1});

			const sl::uint32_t copy_region_count = std::min(dst_image.mip_level_count(), static_cast<sl::uint32_t>(max_mip_levels));
			std::unique_ptr<VkBufferImageCopy[]> copy_regions = std::make_unique_for_overwrite<VkBufferImageCopy[]>(copy_region_count);
			for(sl::uint32_t j = 0; j < copy_region_count; ++j) {
				new (&copy_regions[j]) VkBufferImageCopy{
					texture_data_infos[i].offset + texture_data_infos[i].mip_offsets[j],
					0, 0,
					VkImageSubresourceLayers{
					    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					    .mipLevel = j,
					    .baseArrayLayer = 0,
					    .layerCount = dst_image.layer_count(),
					},
					VkOffset3D{},
					VkExtent3D{
						.width = texture_data_infos[i].extent.width() >> j,
						.height = texture_data_infos[i].extent.height() >> j,
						.depth = 1
					}
				};
			}

			sl::invoke(proc.vulkan_functions_ptr()->vkCmdCopyBufferToImage,
				transfer_command_buffer, 
				static_cast<VkBuffer>(src),
				*dst_image.handle_ref(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				copy_region_count, copy_regions.get()
			);

			sl::array<1, VkImageMemoryBarrier2> post_copy_barriers{{
				VkImageMemoryBarrier2{
				    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
					.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
					.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
					.dstStageMask = VK_PIPELINE_STAGE_2_NONE,
					.dstAccessMask = VK_ACCESS_2_NONE,
				    .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				    .newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
				    .image = *dst_image.handle_ref(),
				    .subresourceRange = {
				        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				        .baseMipLevel = 0,
				        .levelCount = dst_image.mip_level_count(),
				        .baseArrayLayer = 0,
				        .layerCount = dst_image.layer_count(),
				    },
				},
			}};
			transfer_command_buffer.pipeline_barrier({}, {}, post_copy_barriers);

			dst_image.current_layout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
		}
		
		return proc.end_dedicated_copy(post_copy_wait_value, timeline::impl::dedicated_command_group::out_of_timeline_copy, timeout);
	}
}




namespace acma {
	template<buffer_config Config, typename RenderProcessT>
	constexpr result<vk::buffer_allocation_unique_ptr> gpu_allocate(
		RenderProcessT const& process,
		sl::constant_type<buffer_config, Config>
	) noexcept {
		constexpr buffer_usage_policy_flags_t usage = Config.usage;
		constexpr VkFlags all_direct_flags  = (~static_cast<VkFlags>(0) >> (std::numeric_limits<VkFlags>::digits - (buffer_usage_policy::num_direct_usage_polcies)));
		constexpr VkFlags all_indirect_flags = (~static_cast<VkFlags>(0) >> (std::numeric_limits<VkFlags>::digits - (buffer_usage_policy::num_indirect_usage_policies))) << buffer_usage_policy::num_direct_usage_polcies;

		vk::buffer_allocation_unique_ptr buff_alloc_ptr{
			new vk::buffer_allocation{{
				.handle{},
				.creation_info{
    			    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
					//clang 21 is bugged - std::max here causes it to crash
					.size = Config.initial_capacity_bytes > 16 ? 
						Config.initial_capacity_bytes : 
						static_cast<sl::size_t>(16),
    			    .usage = (usage & all_direct_flags),
    			    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
				},
			}, {}},
			vk::impl::buffer_allocation_deleter{process.allocator_ptr()}
		};

		buff_alloc_ptr->creation_info.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;


		if (usage & all_indirect_flags)
			buff_alloc_ptr->creation_info.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		// if (usage & buffer_usage_policy::asset_heap_table)
			// ret.flags |= VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT;
		
		
		if (memory_policy::is_cpu_visible(Config.memory)) {
			buff_alloc_ptr->allocation_creation_info.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
			buff_alloc_ptr->allocation_creation_info.flags |= (memory_policy::is_cpu_writable(Config.memory) ? 
				VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT : 
				VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);
		}

		buff_alloc_ptr->allocation_creation_info.usage = (memory_policy::is_directly_gpu_visible(Config.memory) ?
			VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE :
			VMA_MEMORY_USAGE_AUTO_PREFER_HOST);


		if (Config.dedicated_allocation)
			buff_alloc_ptr->allocation_creation_info.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;


		__D2D_VULKAN_VERIFY(vmaCreateBuffer(
			*process.allocator_ptr(),
			&buff_alloc_ptr->creation_info,
			&buff_alloc_ptr->allocation_creation_info,
			&buff_alloc_ptr->handle,
			&buff_alloc_ptr->allocation_handle,
			&buff_alloc_ptr->allocation_info
		));

		const VkBufferDeviceAddressInfo device_address_info{
			.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR,
			.buffer = buff_alloc_ptr->handle
		};
		buff_alloc_ptr->device_address = sl::invoke(process.vulkan_functions_ptr()->vkGetBufferDeviceAddress, *process.logical_device_ptr(), &device_address_info);

		return buff_alloc_ptr;
	}


	template<typename RenderProcessT>
	constexpr result<vk::buffer_allocation_unique_ptr> gpu_allocate_like(
		RenderProcessT const& process,
		vk::buffer_allocation_unique_ptr const& old_allocation,
		sl::size_t new_size_bytes
	) noexcept {
		vk::buffer_allocation_unique_ptr buff_alloc_ptr(
			new vk::buffer_allocation{*old_allocation},
			vk::impl::buffer_allocation_deleter{process.allocator_ptr()}
		);

		buff_alloc_ptr->creation_info.size = new_size_bytes;

		__D2D_VULKAN_VERIFY(vmaCreateBuffer(
			*process.allocator_ptr(),
			&buff_alloc_ptr->creation_info,
			&buff_alloc_ptr->allocation_creation_info,
			&buff_alloc_ptr->handle,
			&buff_alloc_ptr->allocation_handle,
			&buff_alloc_ptr->allocation_info
		));

		const VkBufferDeviceAddressInfo device_address_info{
			.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR,
			.buffer = buff_alloc_ptr->handle
		};
		buff_alloc_ptr->device_address = sl::invoke(process.vulkan_functions_ptr()->vkGetBufferDeviceAddress, *process.logical_device_ptr(), &device_address_info);
		
        return buff_alloc_ptr;
	}
}

namespace acma {
	constexpr result<vk::image_allocation_unique_ptr> gpu_allocate(
		sl::reference_ptr<const vk::allocator> allocator_ptr,
		vk::image_creation_info_t create_info,
		bool dedicated_allocation
	) noexcept {
		vk::image_allocation_unique_ptr img_alloc_ptr(
			new vk::image_allocation{
				.handle{},
				.creation_info = create_info,
			},
			vk::impl::image_allocation_deleter{allocator_ptr}
		);

		img_alloc_ptr->allocation_creation_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

		if(dedicated_allocation)
			img_alloc_ptr->allocation_creation_info.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

		__D2D_VULKAN_VERIFY(vmaCreateImage(
			*allocator_ptr,
			&img_alloc_ptr->creation_info,
			&img_alloc_ptr->allocation_creation_info,
			&img_alloc_ptr->handle,
			&img_alloc_ptr->allocation_handle,
			&img_alloc_ptr->allocation_info
		));
		
        return img_alloc_ptr;
	}
}
