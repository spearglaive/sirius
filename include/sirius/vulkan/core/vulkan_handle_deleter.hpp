#pragma once
#include <streamline/functional/invoke.hpp>


namespace acma::vk::impl {
	template<typename HandleT, typename DeleterFnPtrT, typename DependentPtrT, typename AuxPtrT>
	struct vulkan_handle_deleter  {
		DeleterFnPtrT deleter_fn_ptr;
		DependentPtrT dependent_ptr;
		AuxPtrT aux_ptr;
	public:
		constexpr void operator()(HandleT h) noexcept {
			sl::invoke(this->deleter_fn_ptr, *dependent_ptr, *aux_ptr, 1, &h);
		}
	};

	template<typename HandleT, typename DeleterFnPtrT, typename DependentPtrT>
	struct vulkan_handle_deleter<HandleT, DeleterFnPtrT, DependentPtrT, sl::empty_t>  {
		DeleterFnPtrT deleter_fn_ptr;
		DependentPtrT dependent_ptr;
	public:
		constexpr void operator()(HandleT h) noexcept {
			sl::invoke(this->deleter_fn_ptr, *dependent_ptr, h, nullptr);
		}
	};

	template<typename HandleT, typename DeleterFnPtrT>
	struct vulkan_handle_deleter<HandleT, DeleterFnPtrT, sl::empty_t, sl::empty_t>  {
		DeleterFnPtrT deleter_fn_ptr;
	public:
		constexpr void operator()(HandleT h) noexcept {
			sl::invoke(this->deleter_fn_ptr, h, nullptr);
		}
	};

	template<typename HandleT>
	struct vulkan_handle_deleter<HandleT, sl::empty_t, sl::empty_t, sl::empty_t>  {
	public:
		constexpr void operator()(HandleT) noexcept {}
	};
}