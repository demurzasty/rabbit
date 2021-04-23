#pragma once

#include "fwd.hpp"

namespace rb {
    template<typename T>
    struct mat3 {
        /**
         * @brief Returns an instance of the identity matrix.
         *
         * Except for the main diagonal, whose elements are all ones,
         * the identity matrix is a square matrix composed of zeros.
         * Unlike all other matrix multiplications, the multiplication
         * between an identity matrix and another matrix is commutative.
         *
         * This is useful for initializing a matrix to a known value
         * before you perform additional operations, such as a transformation.
         *
         * @return Identity matrix.
         */
        static constexpr mat3<T> identity() {
            return {
                1, 0, 0,
                0, 1, 0,
                0, 0, 1
            };
        }

        T values[9];
    };
}
