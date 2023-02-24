#pragma once 

#include "vec2.hpp"

namespace rb {
    /**
     * @brief 2×3 matrix (2 rows, 3 columns) used for 2D linear transformations.
     */
    template<typename T>
    struct basic_mat2x3 {
        /** 
         * @brief Create a identity matrix.
         */
        static constexpr basic_mat2x3 identity() { return { { { 1.0f, 0.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f } } }; }

        /**
         * @brief Create a translation matrix.
         */
        static constexpr basic_mat2x3 translation(const basic_vec2<T>& position) { return { { { 1.0f, 0.0f }, { 0.0f, 1.0f }, position } }; }

        /**
         * @brief Create a translation matrix.
         * 
         * @param scale Scale.
         */
        static constexpr basic_mat2x3 scaling(const basic_vec2<T>& scale) { return { { { scale.x, 0.0f }, { 0.0f, scale.y }, { 0.0f, 0.0f } } }; }

        /**
         * @brief Create a rotation matrix.
         * 
         * @param rotation Angle in radians.
         */
        static inline basic_mat2x3 rotation(float rotation) { 
            float cr = std::cos(rotation);
            float sr = std::sin(rotation);

            return { { { cr, sr }, { -sr, cr }, { 0.0f, 0.0f } } };
        }

        /**
         * @brief Get column using an index. 
         * 
         * @return Column according to index.
        */
        constexpr const basic_vec2<T>& operator[](std::size_t index) const { return cols[index]; }

        /**
         * @brief Get column using an index.
         *
         * @return Column according to index.
        */
        constexpr basic_vec2<T>& operator[](std::size_t index) { return cols[index]; }

        /**
         * @brief Determines the product of two martices.
         * 
         * @brief matrix The second matrix to multiply.
         * 
         * @return The product of the two matrices.
         */
        inline basic_mat2x3<T> operator*(const basic_mat2x3<T>& matrix) const {
            return { {
                { cols[0][0] * matrix[0][0] + cols[0][1] * matrix[1][0],                cols[0][0] * matrix[0][1] + cols[0][1] * matrix[1][1] },
                { cols[1][0] * matrix[0][0] + cols[1][1] * matrix[1][0],                cols[1][0] * matrix[0][1] + cols[1][1] * matrix[1][1] },
                { cols[2][0] * matrix[0][0] + cols[2][1] * matrix[1][0] + matrix[2][0], cols[2][0] * matrix[0][1] + cols[2][1] * matrix[1][1] + matrix[2][1]},
            } };
        }

        /**
         * @brief Calculates the determinant of this matrix.
         * 
         * @return Result of the determinant.
         */
        inline T determinant() const {
            return cols[0][0] * cols[1][1] - cols[0][1] * cols[1][0];
        }

        /**
         * @brief Calculates the inverse of this matrix.
         * 
         * @return Inversed matrix.
         */
        inline basic_mat2x3<T> inverted() const {
            T det = determinant();
            if (det == 0) {
                return identity();
            }

            T inv_det = 1 / det;

            float off_x = cols[2][0];
            float off_y = cols[2][1];

            return { {
                { cols[1][1] * inv_det, -cols[0][1] * inv_det },
                { -cols[1][0] * inv_det, cols[0][0] * inv_det },
                { (cols[1][0] * off_y - off_x * cols[1][1]) * inv_det, (cols[0][1] * off_x - cols[0][0] * off_y) * inv_det },
            } };
        }

        /**
         * @brief Columns.
         */
        basic_vec2<T> cols[3];
    };

    using mat2x3 = basic_mat2x3<float>;
}
