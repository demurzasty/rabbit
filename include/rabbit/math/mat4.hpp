#pragma once

#include "fwd.hpp"
#include "vec3.hpp"
#include "vec4.hpp"

#include <cmath>
#include <cstddef>

namespace rb {
    template<typename T>
    struct mat4 {
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
        static constexpr mat4<T> identity() {
            return {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            };
        }

        /**
         * @brief Builds an orthogonal projection matrix.
         *
         * Projection space refers to the space after applying projection
         * transformation from view space. After the projection transformation,
         * visible content has x and y coordinates ranging from -1 to 1,
         * and z coordinates ranging from 0 to 1.
         *
         * Unlike perspective projection, in orthographic projection there is no
         * perspective foreshortening.
         *
         * The viewable area of this orthographic projection extends from left to
         * right on the x-axis, bottom to top on the y-axis, and near to far on the z-axis.
         * These values are relative to the position and x, y, and z-axes of the view.
         *
         * @param left Minimum x-value of the view volume.
         * @param right Maximum x-value of the view volume.
         * @param bottom Minimum y-value of the view volume.
         * @param top Maximum y-value of the view volume.
         * @param near Minimum z-value of the view volume.
         * @param far Maximum z-value of the view volume.
         *
         * @return Orthographic matrix.
         */
        static constexpr mat4<T> orthographic(T left, T right, T bottom, T top, T near, T far) {
            const auto a = -((right + left) / (right - left));
            const auto b = -((top + bottom) / (top - bottom));
            const auto c = -((far + near) / (far - near));

            return {
                2 / (right - left), 0, 0, 0,
                0, 2 / (top - bottom), 0, 0,
                0, 0, -2 / (far - near), 0,
                a, b, c, 1
            };
        }

        /**
         * @brief Builds a perspective projection matrix based on a field of view.
         *
         * Projection space refers to the space after applying projection transformation
         * from view space. After the projection transformation, visible content
         * has x- and y-coordinates ranging from −1 to 1, and a z-coordinate ranging from 0 to 1.
         *
         * @param fovy Field of view in the y direction, in radians.
         * @param aspect Aspect ration, defined as view space width divided by height.
         * @param near Distance to the near view plane.
         * @param far Distance to the far view plane.
         *
         * @return Perspective projection matrix.
         */
        static mat4<T> perspective(T fovy, T aspect, T near, T far) {
            const auto r = fovy / 2;
            const auto delta = near - far;
            const auto s = std::sin(r);

            if (delta == 0 || s == 0 || aspect == 0) {
                return mat4<T>::identity();
            }

            const auto cotangent = std::cos(r) / s;

            return {
                cotangent / aspect, 0, 0, 0,
                0, cotangent, 0, 0,
                0, 0, (near + far) / delta, -1,
                0, 0, (2 * near * far) / delta, 0
            };
        }

        /**
         * @brief Creates a scaling matrix.
         *
         * @param scale Amounts to scale by on the x, y and z axes.
         *
         * @return Scaling matrix.
         */
        static constexpr mat4<T> scaling(const vec3<T>& scale) {
            return {
                scale.x, 0, 0, 0,
                0, scale.y, 0, 0,
                0, 0, scale.z, 0,
                0, 0, 0, 1
            };
        }

        /**
         * @brief Creates a translation matrix.
         *
         * @param position Amounts to translate by on the x, y and z axes.
         *
         * @return Translation matrix.
         */
        static constexpr mat4<T> translation(const vec3<T>& position) {
            return {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                position.x, position.y, position.z, 1
            };
        }

        /**
         * @brief Creates a matrix that can be used to rotate a set of vertices around the x-axis.
         *
         * @param radians The amount, in radians, in which to rotate around the x-axis.
         *
         * @return Matrix that can be used to rotate a set of vertices around the x-axis.
         */
        static mat4<T> rotation_x(T radians) {
            const auto c = std::cos(radians);
            const auto s = std::sin(radians);

            return {
                1, 0, 0, 0,
                0, c, s, 0,
                0, -s, c, 0,
                0, 0, 0, 1
            };
        }

        /**
         * @brief Creates a matrix that can be used to rotate a set of vertices around the y-axis.
         *
         * @param radians The amount, in radians, in which to rotate around the y-axis.
         *
         * @return Matrix that can be used to rotate a set of vertices around the y-axis.
         */
        static mat4<T> rotation_y(T radians) {
            const auto c = std::cos(radians);
            const auto s = std::sin(radians);

            return {
                c, 0, -s, 0,
                0, 1, 0, 0,
                s, 0, c, 0,
                0, 0, 0, 1
            };
        }

        /**
         * @brief Creates a matrix that can be used to rotate a set of vertices around the z-axis.
         *
         * @param radians The amount, in radians, in which to rotate around the z-axis.
         *
         * @return Matrix that can be used to rotate a set of vertices around the z-axis.
         */
        static mat4<T> rotation_z(T radians) {
            const auto c = std::cos(radians);
            const auto s = std::sin(radians);

            return {
                c, s, 0, 0,
                -s, c, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            };
        }

        /**
         * @brief Creates a new rotation matrix from a specified yaw, pitch, and roll.
         *
         * @param rotation The amount, in radians, in which to rotate around the axes.
         *
         * @return A new rotation matrix with the specified yaw, pitch, and roll.
         */
        static mat4<T> rotation(const vec3<T>& radians) {
            return rotation_x(radians.x) * rotation_z(radians.z) * rotation_y(radians.y);
        }

        constexpr const T& operator[](std::size_t index) const {
            return values[index];
        }

        constexpr T& operator[](std::size_t index) {
            return values[index];
        }

        T values[16];
    };

    /**
     * @brief Calculates the inverse of a matrix.
     *
     * @param mat Source matrix.
     *
     * @return The inverse of the matrix.
     */
    template<typename T>
    constexpr mat4<T> invert(const mat4<T>& mat) {
        const mat4<T> out = {
            mat[5] * mat[10] * mat[15] -
            mat[5] * mat[11] * mat[14] -
            mat[9] * mat[6] * mat[15] +
            mat[9] * mat[7] * mat[14] +
            mat[13] * mat[6] * mat[11] -
            mat[13] * mat[7] * mat[10],

            -mat[1] * mat[10] * mat[15] +
            mat[1] * mat[11] * mat[14] +
            mat[9] * mat[2] * mat[15] -
            mat[9] * mat[3] * mat[14] -
            mat[13] * mat[2] * mat[11] +
            mat[13] * mat[3] * mat[10],

            mat[1] * mat[6] * mat[15] -
            mat[1] * mat[7] * mat[14] -
            mat[5] * mat[2] * mat[15] +
            mat[5] * mat[3] * mat[14] +
            mat[13] * mat[2] * mat[7] -
            mat[13] * mat[3] * mat[6],

            -mat[1] * mat[6] * mat[11] +
            mat[1] * mat[7] * mat[10] +
            mat[5] * mat[2] * mat[11] -
            mat[5] * mat[3] * mat[10] -
            mat[9] * mat[2] * mat[7] +
            mat[9] * mat[3] * mat[6],

            -mat[4] * mat[10] * mat[15] +
            mat[4] * mat[11] * mat[14] +
            mat[8] * mat[6] * mat[15] -
            mat[8] * mat[7] * mat[14] -
            mat[12] * mat[6] * mat[11] +
            mat[12] * mat[7] * mat[10],

            mat[0] * mat[10] * mat[15] -
            mat[0] * mat[11] * mat[14] -
            mat[8] * mat[2] * mat[15] +
            mat[8] * mat[3] * mat[14] +
            mat[12] * mat[2] * mat[11] -
            mat[12] * mat[3] * mat[10],

            -mat[0] * mat[6] * mat[15] +
            mat[0] * mat[7] * mat[14] +
            mat[4] * mat[2] * mat[15] -
            mat[4] * mat[3] * mat[14] -
            mat[12] * mat[2] * mat[7] +
            mat[12] * mat[3] * mat[6],

            mat[0] * mat[6] * mat[11] -
            mat[0] * mat[7] * mat[10] -
            mat[4] * mat[2] * mat[11] +
            mat[4] * mat[3] * mat[10] +
            mat[8] * mat[2] * mat[7] -
            mat[8] * mat[3] * mat[6],

            mat[4] * mat[9] * mat[15] -
            mat[4] * mat[11] * mat[13] -
            mat[8] * mat[5] * mat[15] +
            mat[8] * mat[7] * mat[13] +
            mat[12] * mat[5] * mat[11] -
            mat[12] * mat[7] * mat[9],

            -mat[0] * mat[9] * mat[15] +
            mat[0] * mat[11] * mat[13] +
            mat[8] * mat[1] * mat[15] -
            mat[8] * mat[3] * mat[13] -
            mat[12] * mat[1] * mat[11] +
            mat[12] * mat[3] * mat[9],

            mat[0] * mat[5] * mat[15] -
            mat[0] * mat[7] * mat[13] -
            mat[4] * mat[1] * mat[15] +
            mat[4] * mat[3] * mat[13] +
            mat[12] * mat[1] * mat[7] -
            mat[12] * mat[3] * mat[5],

            -mat[0] * mat[5] * mat[11] +
            mat[0] * mat[7] * mat[9] +
            mat[4] * mat[1] * mat[11] -
            mat[4] * mat[3] * mat[9] -
            mat[8] * mat[1] * mat[7] +
            mat[8] * mat[3] * mat[5],

            -mat[4] * mat[9] * mat[14] +
            mat[4] * mat[10] * mat[13] +
            mat[8] * mat[5] * mat[14] -
            mat[8] * mat[6] * mat[13] -
            mat[12] * mat[5] * mat[10] +
            mat[12] * mat[6] * mat[9],

            mat[0] * mat[9] * mat[14] -
            mat[0] * mat[10] * mat[13] -
            mat[8] * mat[1] * mat[14] +
            mat[8] * mat[2] * mat[13] +
            mat[12] * mat[1] * mat[10] -
            mat[12] * mat[2] * mat[9],

            -mat[0] * mat[5] * mat[14] +
            mat[0] * mat[6] * mat[13] +
            mat[4] * mat[1] * mat[14] -
            mat[4] * mat[2] * mat[13] -
            mat[12] * mat[1] * mat[6] +
            mat[12] * mat[2] * mat[5],

            mat[0] * mat[5] * mat[10] -
            mat[0] * mat[6] * mat[9] -
            mat[4] * mat[1] * mat[10] +
            mat[4] * mat[2] * mat[9] +
            mat[8] * mat[1] * mat[6] -
            mat[8] * mat[2] * mat[5]
        };

        const auto det = mat[0] * out[0] + mat[1] * out[4] + mat[2] * out[8] + mat[3] * out[12];
        const auto inv_det = det != 0 ? (1 / det) : 0;

        return {
            out[0] * inv_det, out[1] * inv_det, out[2] * inv_det, out[3] * inv_det,
            out[4] * inv_det, out[5] * inv_det, out[6] * inv_det, out[7] * inv_det,
            out[8] * inv_det, out[9] * inv_det, out[10] * inv_det, out[11] * inv_det,
            out[12] * inv_det, out[13] * inv_det, out[14] * inv_det, out[15] * inv_det
        };
    }

    /**
     * @brief Multiplies a matrix by another matrix.
     *
     * @param a Source matrix.
     * @param b Source matrix.
     *
     * @return Result of the multiplication.
     */
    template<typename T>
    constexpr mat4<T> operator*(const mat4<T>& a, const mat4<T>& b) {
        return {
            a[0] * b[0] + a[4] * b[1] + a[8] * b[2] + a[12] * b[3],
            a[1] * b[0] + a[5] * b[1] + a[9] * b[2] + a[13] * b[3],
            a[2] * b[0] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3],
            a[3] * b[0] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3],

            a[0] * b[4] + a[4] * b[5] + a[8] * b[6] + a[12] * b[7],
            a[1] * b[4] + a[5] * b[5] + a[9] * b[6] + a[13] * b[7],
            a[2] * b[4] + a[6] * b[5] + a[10] * b[6] + a[14] * b[7],
            a[3] * b[4] + a[7] * b[5] + a[11] * b[6] + a[15] * b[7],

            a[0] * b[8] + a[4] * b[9] + a[8] * b[10] + a[12] * b[11],
            a[1] * b[8] + a[5] * b[9] + a[9] * b[10] + a[13] * b[11],
            a[2] * b[8] + a[6] * b[9] + a[10] * b[10] + a[14] * b[11],
            a[3] * b[8] + a[7] * b[9] + a[11] * b[10] + a[15] * b[11],

            a[0] * b[12] + a[4] * b[13] + a[8] * b[14] + a[12] * b[15],
            a[1] * b[12] + a[5] * b[13] + a[9] * b[14] + a[13] * b[15],
            a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15],
            a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15]
        };
    }

    template<typename T>
    constexpr vec3<T> operator*(const mat4<T>& a, const vec3<T>& b) {
        const vec4<T> vec = {
            b.x * a[0] + b.y * a[4] + b.z * a[8] + a[12],
            b.x * a[1] + b.y * a[5] + b.z * a[9] + a[13],
            b.x * a[2] + b.y * a[6] + b.z * a[10] + a[14],
            b.x * a[3] + b.y * a[7] + b.z * a[11] + a[15]
        };
        return { vec.x / vec.w, vec.y / vec.w, vec.z / vec.w };
    }

    template<typename T>
    constexpr vec4<T> operator*(const mat4<T>& a, const vec4<T>& b) {
        return {
            b.x * a[0] + b.y * a[4] + b.z * a[8] + b.w * a[12],
            b.x * a[1] + b.y * a[5] + b.z * a[9] + b.w * a[13],
            b.x * a[2] + b.y * a[6] + b.z * a[10] + b.w * a[14],
            b.x * a[3] + b.y * a[7] + b.z * a[11] + b.w * a[15]
        };
    }
}
