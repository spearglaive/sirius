#pragma once
#include <streamline/numeric/int.hpp>
#include <streamline/metaprogramming/integer_sequence.hpp>


namespace acma {
	using descriptor_key_t = sl::uint64_t;
}

namespace acma {
	template<descriptor_key_t... Ks>
	using descriptor_key_sequence_type = sl::integer_sequence_type<descriptor_key_t, Ks...>;
	template<descriptor_key_t... Ks>
	inline constexpr descriptor_key_sequence_type<Ks...> descriptor_key_sequence{};

	template<descriptor_key_t K>
	using descriptor_key_constant_type = sl::constant_type<descriptor_key_t, K>;
	template<descriptor_key_t K>
	inline constexpr descriptor_key_constant_type<K> descriptor_key_constant{};
}
