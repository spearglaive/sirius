#pragma once
#include <streamline/metaprogramming/type_traits/relationships.hpp>

#include "sirius/vulkan/core/vulkan.hpp"


namespace acma::vk {
	template<typename VulkanT, auto DeleteFn>
	class vulkan_handle;
}

namespace acma::vk {
	template<typename FnT, typename VulkanT>
	concept delete_invocable = sl::traits::is_invocable_r_v<FnT, void, VulkanT&, VkAllocationCallbacks const*>;

	template<typename FnT, typename VulkanT>
	concept instance_delete_invocable = sl::traits::is_invocable_r_v<FnT, void, VkInstance, VulkanT&, VkAllocationCallbacks const*>;

	template<typename FnT, typename VulkanT>
	concept device_delete_invocable = sl::traits::is_invocable_r_v<FnT, void, VkDevice, VulkanT&, VkAllocationCallbacks const*>;
	
	template<typename FnT, typename VulkanT>
	concept command_pool_delete_invocable = sl::traits::is_invocable_r_v<FnT, void, VkDevice, VkCommandPool, sl::uint32_t, VulkanT const*>;

	constexpr bool kekoms = instance_delete_invocable<decltype(vkDestroySurfaceKHR), VkSurfaceKHR>;
	constexpr bool pekoms = command_pool_delete_invocable<decltype(vkFreeCommandBuffers), VkCommandBuffer>;
}


namespace acma::vk {
	template<typename VulkanT, auto DeleteFn>
	requires delete_invocable<decltype(DeleteFn), VulkanT>
	class vulkan_handle<VulkanT, DeleteFn> : public VulkanT {
	public:
		vulkan_handle(vulkan_handle const&) = delete;
		vulkan_handle& operator=(vulkan_handle const&) = delete;

	public:
		constexpr vulkan_handle() noexcept = default;	
	public:
		~vulkan_handle() noexcept = default;
	};
}

namespace acma::vk {
	template<typename VulkanT, auto DeleteFn>
	requires instance_delete_invocable<decltype(DeleteFn), VulkanT>
	class vulkan_handle<VulkanT, DeleteFn> : public VulkanT {
	public:
		vulkan_handle(vulkan_handle const&) = delete;
		vulkan_handle& operator=(vulkan_handle const&) = delete;

	public:
		constexpr vulkan_handle(VkInstance const& instance) noexcept :
			VulkanT{}, instance_ref(instance) {}
	public:
		~vulkan_handle() noexcept = default;

	private:
		VkInstance const& instance_ref;
	};
}

namespace acma::vk {
	template<typename VulkanT, auto DeleteFn>
	requires device_delete_invocable<decltype(DeleteFn), VulkanT>
	class vulkan_handle<VulkanT, DeleteFn> : public VulkanT {
	public:
		vulkan_handle(vulkan_handle const&) = delete;
		vulkan_handle& operator=(vulkan_handle const&) = delete;

	public:
		constexpr vulkan_handle(VkDevice const& device) noexcept :
			VulkanT{}, instance_ref(device) {}
	public:
		~vulkan_handle() noexcept = default;

	private:
		VkDevice const& instance_ref;
	};
}