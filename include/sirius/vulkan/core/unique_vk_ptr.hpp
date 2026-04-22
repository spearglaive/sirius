#pragma once
#include <streamline/memory/unique_ptr.hpp>


namespace acma::vk {
	template<typename MixinT>
	struct unique_vk_ptr : public sl::unique_ptr<sl::remove_pointer_t<typename MixinT::handle_type>, typename MixinT::deleter_type> {
		using mixin_type = MixinT;
		using handle_type                    = typename mixin_type::handle_type;
		using deletion_function_pointer_type = typename mixin_type::deletion_function_pointer_type;
		using deletion_dependent_type        = typename mixin_type::deletion_dependent_type;
		using deletion_auxiliary_type        = typename mixin_type::deletion_auxiliary_type;
		using deleter_type                   = typename mixin_type::deleter_type;
		using base_type = sl::unique_ptr<sl::remove_pointer_t<handle_type>, deleter_type>; 
		

	public:
		constexpr unique_vk_ptr() noexcept = default;
		constexpr ~unique_vk_ptr() noexcept = default;

		constexpr unique_vk_ptr(unique_vk_ptr&&) noexcept = default;
		constexpr unique_vk_ptr& operator=(unique_vk_ptr&&) noexcept = default;

		constexpr unique_vk_ptr(unique_vk_ptr const&) noexcept = delete;
		constexpr unique_vk_ptr& operator=(unique_vk_ptr const&) noexcept = delete;
	public:
		constexpr unique_vk_ptr(
			deletion_function_pointer_type fn,
			handle_type h = nullptr
		) noexcept
		requires(
			mixin_type::has_function_type &&
			!mixin_type::has_dependent_type &&
			!mixin_type::has_aux_type
		) :
			base_type{h, deleter_type{fn}} {}


		constexpr unique_vk_ptr(
			deletion_function_pointer_type fn,
			deletion_dependent_type dependent_ptr,
			handle_type h = nullptr
		) noexcept
		requires(
			mixin_type::has_function_type &&
			mixin_type::has_dependent_type &&
			!mixin_type::has_aux_type
		) :
			base_type{h, deleter_type{fn, dependent_ptr}} {}


		constexpr unique_vk_ptr(
			deletion_function_pointer_type fn,
			deletion_dependent_type dependent_ptr,
			deletion_auxiliary_type aux_ptr,
			handle_type h = nullptr
		) noexcept
		requires(
			mixin_type::has_function_type &&
			mixin_type::has_dependent_type &&
			mixin_type::has_aux_type
		) :
			base_type{h, deleter_type{fn, dependent_ptr, aux_ptr}} {}
	};
}