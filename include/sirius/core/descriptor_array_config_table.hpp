#pragma once
#include <cstddef>
#include <streamline/containers/lookup_table.hpp>
#include <streamline/universal/make_deduced.hpp>
#include <streamline/functional/functor/subscript.hpp>

#include "sirius/core/descriptor_array_config.hpp"
#include "sirius/core/descriptor_key_t.hpp"


namespace acma {
    template<std::size_t M>
    using descriptor_array_config_table = sl::lookup_table<M, descriptor_key_t, descriptor_array_config>;
}


namespace acma::impl {
	template<typename>
	struct is_descriptor_array_config_table : sl::false_constant_type {};

	template<sl::size_t M>
	struct is_descriptor_array_config_table<descriptor_array_config_table<M>> : sl::true_constant_type {};


    template<typename T>
	constexpr bool is_descriptor_array_config_table_v = is_descriptor_array_config_table<T>::value;
}
