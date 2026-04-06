#pragma once 
#include <mutex>
#include <functional>
#include <streamline/containers/array.hpp>

#include "sirius/input/combination.hpp"
#include "sirius/input/category.hpp"
#include "sirius/input/map_types.hpp"
#include "sirius/input/event_function.hpp"
#include "sirius/input/modifier_flags.hpp"


namespace acma::input {
    struct info {
        category_flags_t category_flags;
        binding_map active_bindings;
        binding_map inactive_bindings;
        event_fns_map event_fns;
        text_event_function_type* text_input_fn;
        sl::array<num_codes, modifier_flags_t> modifier_flags;

        combination current_combo;
		std::mutex current_combo_mutex;
    };
}