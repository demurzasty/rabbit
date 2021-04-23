#pragma once

#include "fwd.hpp"

namespace rb {
    template<typename T>
    struct mat3 {
        static constexpr mat4<T> identity() {
            return {
                1, 0, 0,
                0, 1, 0,
                0, 0, 1
            };
        }

        T values[9];
    };
}
