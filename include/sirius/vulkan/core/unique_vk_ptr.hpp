#pragma once
#include <streamline/metaprogramming/type_traits/isolated_categories.hpp>


namespace acma::vk {
	template<typename T>
	struct unique_handle {
		


		typename T::handle_type _handle;
	};
}