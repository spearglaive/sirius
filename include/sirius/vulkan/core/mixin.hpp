#pragma once
#include <memory>

#include <streamline/memory/unique_ptr.hpp>
#include <streamline/metaprogramming/type_traits/relationships.hpp>
#include <streamline/metaprogramming/conditional.hpp>
#include <streamline/metaprogramming/empty_t.hpp>

#include "sirius/vulkan/core/unique_vk_ptr.hpp"
#include "sirius/vulkan/core/vulkan_handle_deleter.hpp"


namespace acma::vk {
	template<typename HandleT, typename DeleterFnT = sl::empty_t, typename DependentT = sl::empty_t, typename AuxT = sl::empty_t>
	struct mixin {
	public:
		constexpr static bool has_function_type = !sl::traits::is_same_as_v<DeleterFnT, sl::empty_t>;
		constexpr static bool has_dependent_type = !sl::traits::is_same_as_v<DependentT, sl::empty_t>;
		constexpr static bool has_aux_type = !sl::traits::is_same_as_v<AuxT, sl::empty_t>;
	public:
		using handle_type = HandleT;
		using deletion_function_pointer_type = sl::conditional_t<has_function_type , DeleterFnT                   , sl::empty_t>;
		using deletion_dependent_type        = sl::conditional_t<has_dependent_type, sl::reference_ptr<const DependentT>, sl::empty_t>;
		using deletion_auxiliary_type        = sl::conditional_t<has_aux_type      , sl::reference_ptr<const AuxT      >, sl::empty_t>;
		using deleter_type = impl::vulkan_handle_deleter<handle_type, deletion_function_pointer_type, deletion_dependent_type, deletion_auxiliary_type>;

	public:
		constexpr operator handle_type() const noexcept { return smart_handle.get(); }
	public:
		constexpr handle_type const* operator&() const& noexcept { return &smart_handle.get(); }
		constexpr handle_type      * operator&()      & noexcept { return &smart_handle.get(); }
		
	public:
		vk::unique_vk_ptr<mixin<HandleT, DeleterFnT, DependentT, AuxT>> smart_handle;
	};
}