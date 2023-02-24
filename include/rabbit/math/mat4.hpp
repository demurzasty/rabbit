#pragma once 

#include "vec3.hpp"
#include "vec4.hpp"

namespace rb {
    template<typename T>
    struct basic_mat4 {
        /**
         * @brief Make identity matrix.
         */
        static constexpr basic_mat4<T> identity() noexcept {
            return { { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } } };
        }

        /**
         * @brief Mirroring applied perpendicular to the YZ plane.
         */
        static constexpr basic_mat4<T> flip_x() noexcept {
            return { { { -1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } } };
        }

        /**
         * @brief Mirroring applied perpendicular to the XZ plane.
         */
        static constexpr basic_mat4<T> flip_y() noexcept {
            return { { { 1, 0, 0, 0 }, { 0, -1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } } };
        }

        /**
         * @brief Mirroring applied perpendicular to the XY plane.
         */
        static constexpr basic_mat4<T> flip_z() noexcept {
            return { { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, -1, 0 }, { 0, 0, 0, 1 } } };
        }

        /**
         * @brief Get column.
         */
        constexpr const basic_vec4<T>& operator[](std::size_t index) const {
            return cols[index];
        }

        /**
         * @brief Get column.
         */
        constexpr basic_vec4<T>& operator[](std::size_t index) {
            return cols[index];
        }

        /**
         * @brief Extract position.
         */
        const basic_vec3<T>& extract_position() const {
            return reinterpret_cast<const basic_vec3<T>&>(cols[3]);
        }

        /**
         * @brief Columns.
         */
        basic_vec4<T> cols[4];
    };

    using mat4 = basic_mat4<float>;
}
