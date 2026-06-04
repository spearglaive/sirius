#pragma once

#include <BS_thread_pool.hpp>

#include "sirius/core/api.def.h"


namespace acma {
    SIRIUS_API BS::priority_thread_pool& thread_pool();
}
