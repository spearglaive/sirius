#pragma once
#include "sirius/core/render_instance.hpp"

#include <cstring>
#include <cstdlib>
#include <memory>
#include <string_view>
#include <utility>
#include <streamline/functional/functor/generic_stateless.hpp>
#include <streamline/functional/functor/subscript.hpp>
#include <streamline/functional/functor/default_construct.hpp>
#include <streamline/functional/functor/invoke_each_result.hpp>
#include <streamline/functional/functor/forward_construct.hpp>

#include "sirius/vulkan/core/vulkan.hpp"

#include <GLFW/glfw3.h>
#include <result/verify.h>


#include "sirius/timeline/command.fwd.hpp"
#include "sirius/core/invoke_all.def.hpp"
#include "sirius/core/error.hpp"
#include "sirius/core/thread_pool.hpp"
#include "sirius/input/codes_map.hpp"
#include "sirius/timeline/state.hpp"
#include "sirius/input/combination.hpp"
#include "sirius/input/event_function.hpp"
#include "sirius/input/event_int.hpp"
#include "sirius/vulkan/core/command_buffer.hpp"
#include "sirius/vulkan/display/surface.hpp"
#include "sirius/core/make.hpp"
#include "sirius/vulkan/sync/semaphore.hpp"


namespace acma::impl {
    template<typename... TimelineEventTs, auto BufferConfigs, auto DescriptorArrayConfigs, sl::size_t UserByteCount> requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
	result<sl::unique_ptr<render_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, DescriptorArrayConfigs, UserByteCount>>>
	make<sl::unique_ptr<render_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, DescriptorArrayConfigs, UserByteCount>>>::
	operator()(
		vk::physical_device& device,
		bool prefer_synchronous_rendering,
		sl::in_place_adl_tag_type<sl::unique_ptr<render_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, DescriptorArrayConfigs, UserByteCount>>>
	) const noexcept {
  		sl::unique_ptr<value_type> ret(new value_type);
		ret->has_window = false;
		RESULT_VERIFY(ret->initialize(sl::false_constant, device, prefer_synchronous_rendering));

		RESULT_VERIFY(ret->initialize_auxiliary());
		return sl::move(ret);
	}
}

namespace acma::impl {
    template<typename... TimelineEventTs, auto BufferConfigs, auto DescriptorArrayConfigs, sl::size_t UserByteCount> requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
	result<sl::unique_ptr<render_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, DescriptorArrayConfigs, UserByteCount>>>
	make<sl::unique_ptr<render_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, DescriptorArrayConfigs, UserByteCount>>>::
	operator()(
		vk::physical_device& device,
		bool prefer_synchronous_rendering,
		acma::sz2u32 window_size,
		std::string_view window_title,
		sl::in_place_adl_tag_type<sl::unique_ptr<render_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, DescriptorArrayConfigs, UserByteCount>>>
	) const noexcept {
		static_assert(impl::window_capability, "Cannot make a render_instance with a window when window capabilites are disabled.");

	    sl::unique_ptr<value_type> ret(new value_type);
		ret->has_window = true;
		RESULT_VERIFY(ret->initialize(sl::true_constant, device, prefer_synchronous_rendering));

		//Create window
		RESULT_TRY_MOVE(static_cast<window&>(*ret), acma::make<window>(
			window_size,
			window_title.empty() ? impl::name() : window_title
		));
		RESULT_VERIFY(static_cast<window&>(*ret).initialize(
			ret->vulkan_functions_ptr(),
			ret->logical_device_ptr(),
			ret->physical_device_ptr(),
			ret->allocator_ptr()
		));

		//Create swap chain sempahores
		ret->_graphics_semaphores.reserve(ret->_swap_chain.image_count());
		ret->_pre_present_semaphores.reserve(ret->_swap_chain.image_count());
		for(std::size_t i = 0; i < ret->_swap_chain.image_count(); ++i) {
			RESULT_VERIFY_UNSCOPED(acma::make<vk::semaphore>(ret->vulkan_functions_ptr(), ret->logical_device_ptr()), graphics_semaphore);
			ret->_graphics_semaphores.push_back(*std::move(graphics_semaphore));

			RESULT_VERIFY_UNSCOPED(acma::make<vk::semaphore>(ret->vulkan_functions_ptr(), ret->logical_device_ptr()), pre_present_semaphore);
			ret->_pre_present_semaphores.push_back(*std::move(pre_present_semaphore));
		}


		D2D_INVOKE_ALL(ret->timeline_callbacks(), on_swap_chain_updated, *ret, *ret, ret->external_timeline_state());

		RESULT_VERIFY(ret->initialize_auxiliary());
		return sl::move(ret);
	}

}


namespace acma {
	template<typename... TimelineEventTs, auto BufferConfigs, auto DescriptorArrayConfigs, sl::size_t UserByteCount> requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
	template<bool Windowing>
	result<void>     render_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, DescriptorArrayConfigs, UserByteCount>::
	initialize(
		sl::bool_constant_type<Windowing>,
		vk::physical_device& device,
		bool prefer_synchronous_rendering
	) noexcept {
		//Set open flag
		should_be_open = std::make_unique<std::atomic<bool>>(true);


		//Initialize physical device
		this->_physical_device_ptr = sl::reference_ptr<vk::physical_device>{std::addressof(device)};
		RESULT_VERIFY(this->_physical_device_ptr->initialize_queues(prefer_synchronous_rendering, Windowing));

		{
		//Create logical device
		VkDevice logical_device_handle = VK_NULL_HANDLE;
		RESULT_TRY_COPY(logical_device_handle, make_device_handle(this->physical_device_ptr(), Windowing));

		//Initialize function pointers
		make_function_table(this->_vulkan_functions, logical_device_handle);

		//Initialize logical device
        RESULT_TRY_MOVE(this->_logical_device, acma::make<vk::logical_device>(
			this->vulkan_functions_ptr(),
			this->physical_device_ptr(),
			Windowing,
			sl::move(logical_device_handle)
		));
		}

		//Create allocator
		{
		VmaAllocatorCreateInfo allocator_create_info {
			.flags =
				VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT |
				VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT |
				VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT |
				VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
			.physicalDevice = *this->physical_device_ptr(),
			.device = *this->logical_device_ptr(),
			.instance = vk::impl::vulkan_instance(),
			.vulkanApiVersion = VK_API_VERSION_1_3,
		};

		VmaVulkanFunctions vma_vk_funcs;
		__D2D_VULKAN_VERIFY(vmaImportVulkanFunctionsFromVolk(&allocator_create_info, &vma_vk_funcs));
		allocator_create_info.pVulkanFunctions = &vma_vk_funcs;

		__D2D_VULKAN_VERIFY(vmaCreateAllocator(&allocator_create_info, &this->_allocator.smart_handle.get()));
		}


		constexpr sl::size_t command_familes_to_init = command_family::num_distinct_families + static_cast<sl::size_t>(impl::window_capability);

		//Create command pools
		for(command_family_t i = 0; i < command_familes_to_init; ++i) {
			RESULT_TRY_MOVE(this->_command_pools[i], acma::make<vk::command_pool>(
				this->vulkan_functions_ptr(),
				this->physical_device_ptr(),
				this->logical_device_ptr(),
				i
			));
		}


		//Initialize buffers
		constexpr auto init_single_buffer = []<sl::index_t I>(
			typename render_instance::buffer_tuple_type& buffer_tuple,
			render_instance* render_inst_ptr,
			sl::index_constant_type<I>
		) noexcept -> result<void> {
			using buffer_type = typename sl::tuple_traits<typename render_instance::buffer_tuple_type>::template type_of_element<I>;
			buffer_tuple[sl::index_constant<I>] = buffer_type{*render_inst_ptr};
			return buffer_tuple[sl::index_constant<I>].initialize();
		};
		RESULT_VERIFY((sl::functor::invoke_each_result<result<void>, init_single_buffer>{}(
			sl::index_sequence_of_length<BufferConfigs.size()>, this->_gpu_buffers, this
		)));


		//Initialize descriptor arrays
		constexpr auto init_single_descriptor_array = []<sl::index_t I>(
			typename render_instance::descriptor_array_tuple_type& descriptor_array_tuple,
			render_instance* render_inst_ptr,
			sl::index_constant_type<I>
		) noexcept -> result<void> {
			using descriptor_array_type = typename sl::tuple_traits<typename render_instance::descriptor_array_tuple_type>::template type_of_element<I>;
			descriptor_array_tuple[sl::index_constant<I>] = descriptor_array_type{*render_inst_ptr};
			return descriptor_array_tuple[sl::index_constant<I>].initialize();
		};
		RESULT_VERIFY((sl::functor::invoke_each_result<result<void>, init_single_descriptor_array>{}(
			sl::index_sequence_of_length<DescriptorArrayConfigs.size()>, this->_asset_descriptor_arrays, this
		)));


		constexpr static sl::size_t dedicated_cmd_buff_count = timeline::impl::dedicated_command_group::num_dedicated_command_groups;
		for(sl::index_t i = 0; i < frames_in_flight; ++i) {
			//Create command buffers
			for(sl::index_t j = 0; j < dedicated_cmd_buff_count; ++j) {
				RESULT_TRY_MOVE(this->_command_buffers[i][j], acma::make<vk::command_buffer>(
					this->vulkan_functions_ptr(),
					this->physical_device_ptr(),
					this->logical_device_ptr(),
					{std::addressof(this->command_pools()[command_family::transfer])}
				));
			}
			for(sl::index_t j = 0; j < command_traits_type::group_count; ++j) {
				if(command_traits_type::group_families[j] == command_family::none) continue;
				RESULT_TRY_MOVE(this->_command_buffers[i][j + dedicated_cmd_buff_count], acma::make<vk::command_buffer>(
					this->vulkan_functions_ptr(),
					this->physical_device_ptr(),
					this->logical_device_ptr(),
					{std::addressof(this->command_pools()[command_traits_type::group_families[j]])}
				));
			}

			//Create command buffer semaphores
			for(sl::index_t j = 0; j < command_group_count; ++j) {
				RESULT_TRY_MOVE(this->_command_buffer_semaphores[i][j], acma::make<vk::semaphore>(this->vulkan_functions_ptr(), this->logical_device_ptr(), VK_SEMAPHORE_TYPE_TIMELINE));
				this->_command_buffer_semaphore_values[i][j] = 0;
			}


			//Create generic semaphores
			for(sl::index_t j = 0; j < command_familes_to_init; ++j) {
				RESULT_TRY_MOVE(this->_command_family_semaphores[i][j], acma::make<vk::semaphore>(this->vulkan_functions_ptr(), this->logical_device_ptr(), VK_SEMAPHORE_TYPE_TIMELINE));
				this->_command_family_semaphore_values[i][j] = 0;
			}

			//Create image acquire semaphore
			RESULT_TRY_MOVE(this->_acquisition_semaphores[i], acma::make<vk::semaphore>(this->vulkan_functions_ptr(), this->logical_device_ptr()));
		}

		return {};
	}


	template<typename... TimelineEventTs, auto BufferConfigs, auto DescriptorArrayConfigs, sl::size_t UserByteCount> requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
	result<void>     render_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, DescriptorArrayConfigs, UserByteCount>::
	initialize_auxiliary() noexcept {
		constexpr auto init_aux = []<sl::index_t I>(render_instance& app_inst, window& win, sl::index_constant_type<I>) noexcept -> result<void> {
			if constexpr (std::is_same_v<typename sl::tuple_traits<decltype(app_inst.auxiliary)>::template type_of_element<I>, sl::empty_t>) {
				//_auxiliary[sl::index_constant<I>] = sl::empty_t{};
				return {};
			}
			RESULT_TRY_MOVE(app_inst.auxiliary[sl::index_constant<I>], (sl::type_of_pack_element_t<I, timeline::setup<TimelineEventTs>...>{}(app_inst, win)));
			return {};
		};
		return sl::functor::invoke_each_result<result<void>, init_aux>{}(sl::index_sequence_for_pack<TimelineEventTs...>, *this, *this);
	}
}


namespace acma {
    template<typename... TimelineEventTs, auto BufferConfigs, auto DescriptorArrayConfigs, sl::size_t UserByteCount> requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
    bool     render_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, DescriptorArrayConfigs, UserByteCount>::
	is_open() const noexcept {
        return should_be_open->load(std::memory_order_relaxed);
    }

    template<typename... TimelineEventTs, auto BufferConfigs, auto DescriptorArrayConfigs, sl::size_t UserByteCount> requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
    void     render_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, DescriptorArrayConfigs, UserByteCount>::
	close() noexcept {
        should_be_open->store(false, std::memory_order_relaxed);
    }


    template<typename... TimelineEventTs, auto BufferConfigs, auto DescriptorArrayConfigs, sl::size_t UserByteCount> requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
    void     render_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, DescriptorArrayConfigs, UserByteCount>::
	poll_events() noexcept {
		static_assert(impl::window_capability, "Cannot poll window events with windowing capabilities disabled");
		if(!has_window) return;

		glfwSetWindowUserPointer(this->window_handle.get(), static_cast<window*>(this));
        glfwPollEvents();

		if(!this->window_handle) [[unlikely]] return;
        if(!glfwWindowShouldClose(this->window_handle.get()))
            return;

        close();
    }


    template<typename... TimelineEventTs, auto BufferConfigs, auto DescriptorArrayConfigs, sl::size_t UserByteCount> requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
    std::future<result<void>>      render_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, DescriptorArrayConfigs, UserByteCount>::
	start_async_render() noexcept {
        return std::async([](render_instance& a) -> acma::result<void> {
            while(a.is_open())
                if(auto r = a.render(); !r.has_value()) [[unlikely]]
                	return r.error();
            return {};
        }, std::ref(*this));
    }

    template<typename... TimelineEventTs, auto BufferConfigs, auto DescriptorArrayConfigs, sl::size_t UserByteCount> requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
    result<void>      render_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, DescriptorArrayConfigs, UserByteCount>::
	join() const noexcept {
        __D2D_VULKAN_VERIFY(sl::invoke(this->vulkan_functions_ptr()->vkDeviceWaitIdle, *this->logical_device_ptr()));
        return {};
    }
}


namespace acma {
	template<typename... TimelineEventTs, auto BufferConfigs, auto DescriptorArrayConfigs, sl::size_t UserByteCount> requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
    result<void> render_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, DescriptorArrayConfigs, UserByteCount>::render() noexcept {
		using filter_dedicated_command_groups_sequence = sl::filtered_sequence_t<
			sl::index_sequence_of_length_type<command_group_count>,
			[]<sl::index_t I>(sl::index_constant_type<I>) noexcept { return I >= acma::timeline::impl::dedicated_command_group::num_dedicated_command_groups; }
		>;
        //wait for rendering to finish last frame
		const sl::index_t frame_idx = this->frame_index();
		const sl::array<command_traits_type::group_count, VkSemaphore> wait_semaphores = sl::universal::make_deduced<sl::generic::array>(
			this->command_buffer_semaphores()[frame_idx],
			sl::functor::forward_construct<VkSemaphore>{},
			filter_dedicated_command_groups_sequence{}
		);
		const sl::array<command_traits_type::group_count, sl::uint64_t> wait_semaphores_values = sl::universal::make_deduced<sl::generic::array>(
			this->command_buffer_semaphore_values()[frame_idx],
			sl::functor::forward_construct<sl::uint64_t>{},
			filter_dedicated_command_groups_sequence{}
		);

		VkSemaphoreWaitInfo wait_info{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.flags = 0,
			.semaphoreCount = command_traits_type::group_count,
			.pSemaphores = wait_semaphores.data(),
			.pValues = wait_semaphores_values.data(),
		};
		__D2D_VULKAN_VERIFY(sl::invoke(this->vulkan_functions_ptr()->vkWaitSemaphores, *this->logical_device_ptr(), &wait_info, std::numeric_limits<std::uint64_t>::max()));


		this->old_allocations()[frame_idx].clear();


		timeline::state<UserByteCount> timeline_state{};

		D2D_INVOKE_ALL(this->timeline_callbacks(), on_frame_begin, *this, *this, timeline_state);


		constexpr auto exec = []<sl::index_t I>(render_instance& app_inst, timeline_state_type& state, sl::index_constant_type<I>) noexcept -> result<void> {
			return app_inst.template execute_command<I>(state);
		};
		RESULT_VERIFY((sl::functor::invoke_each_result<result<void>, exec>{}(sl::index_sequence_for_pack<TimelineEventTs...>, *this, timeline_state)));
        //((Ts{}(*this, timeline_state)), ...);


		D2D_INVOKE_ALL(this->timeline_callbacks(), on_frame_end, *this, *this, timeline_state);

        //frame_idx = (frame_idx + 1) % impl::frames_in_flight;
        ++this->_frame_count;
		//this->frame_count.fetch_add();
        return {};
    }


	template<typename... TimelineEventTs, auto BufferConfigs, auto DescriptorArrayConfigs, sl::size_t UserByteCount> requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
	template<sl::index_t I>
    result<void> render_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, DescriptorArrayConfigs, UserByteCount>::execute_command(timeline_state_type& state) noexcept {
		using timeline_type = typename sl::tuple_traits<sl::tuple<TimelineEventTs...>>::template type_of_element<I>;
		return acma::timeline::command<timeline_type>{}(
			*this,
			*this,
			state,
			sl::universal::get<I>(auxiliary),
			sl::index_constant<command_traits_type::group_indices[I] + timeline::impl::dedicated_command_group::num_dedicated_command_groups>
		);
	}
}

namespace acma {
	template<typename... TimelineEventTs, auto BufferConfigs, auto DescriptorArrayConfigs, sl::size_t UserByteCount> requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
	template<typename TimelineCommandT>
    result<void> render_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, DescriptorArrayConfigs, UserByteCount>::execute_command(timeline_state_type& state) noexcept {
		return acma::timeline::command<TimelineCommandT>{}(
			*this,
			*this,
			state,
			sl::empty_t{},
			timeline::impl::dedicated_command_group::out_of_timeline_execute
		);
	}

	template<typename... TimelineEventTs, auto BufferConfigs, auto DescriptorArrayConfigs, sl::size_t UserByteCount> requires impl::is_buffer_config_table_v<decltype(BufferConfigs)>
	template<typename TimelineCommandT>
    result<void> render_instance<sl::tuple<TimelineEventTs...>, BufferConfigs, DescriptorArrayConfigs, UserByteCount>::execute_command() noexcept {
		return execute_command(external_timeline_state());
	}
}
