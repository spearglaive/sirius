#pragma once
#include <cstdint>

#include "sirius/vulkan/core/vulkan.hpp"
#include "sirius/vulkan/memory/allocator.hpp"


namespace acma {
    using memory_policy_t = std::uint8_t;

	namespace memory_policy {
    enum : memory_policy_t {
        gpu_local, 
        cpu_local_cpu_writable, 
		cpu_local_gpu_writable,
        shared,
        push_constant,

        num_memory_policies,
		num_allocation_backed_memory_policies = num_memory_policies - 1,

    };
	}
}


namespace acma {
	namespace memory_policy {
		constexpr bool is_cpu_visible(memory_policy_t mp) noexcept {
			return mp != memory_policy::gpu_local;
		}

		constexpr bool is_cpu_writable(memory_policy_t mp) noexcept {
			return is_cpu_visible(mp) && mp != memory_policy::cpu_local_gpu_writable;
		}

		constexpr bool is_directly_gpu_visible(memory_policy_t mp) noexcept {
			return !is_cpu_visible(mp) || mp == memory_policy::shared;
		}
	}
}