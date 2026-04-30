#pragma once
#include <optional>

#include "sirius/input/combination.hpp"
#include "sirius/input/event_int.hpp"
#include "sirius/arith/point.hpp"
#include "sirius/core/window.fwd.hpp"


namespace acma::input {
    using mouse_aux_t = std::optional<acma::pt2d>;
}

namespace acma::input{
    using generic_event_function_type = void(window&, combination, bool, categorized_event_t, mouse_aux_t);
    using text_event_function_type = void(window&, unsigned int);
}
