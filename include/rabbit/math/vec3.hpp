#pragma once 

#include "math.hpp"

namespace rb {
    /**
     * @brief Representation of 3D vectors and points.
     */
    template<typename T>
    struct basic_vec3 {
        /**
         * @brief Make 3D vector with both of its components set to zero.
         *
         * @return 3D vector with both of its components set to zero.
         */
        [[nodiscard]] static constexpr basic_vec3<T> zero() { return { 0, 0, 0 }; }

        /**
         * @brief Make 3D vector with both of its components set to one.
         * 
         * @return 3D vector with both of its components set to one.
         */
        [[nodiscard]] static constexpr basic_vec3<T> one() { return { 1, 1, 1 }; }

        /**
         * @brief Make 3D vector with X component set to one.
         *
         * @return 3D vector with X component set to one.
         */
        [[nodiscard]] static constexpr basic_vec3<T> axis_x() { return { 1, 0, 0 }; }

        /**
         * @brief Make 3D vector with Y component set to one.
         *
         * @return 3D vector with Y component set to one.
         */
        [[nodiscard]] static constexpr basic_vec3<T> axis_y() { return { 0, 1, 0 }; }

        /**
         * @brief Make 3D vector with Z component set to one.
         *
         * @return 3D vector with Z component set to one.
         */
        [[nodiscard]] static constexpr basic_vec3<T> axis_z() { return { 0, 0, 1 }; }

        /**
         * @brief Construct a new uninitialized 3D vector.
         */
        basic_vec3() = default;

        /**
         * @brief Construct a new 3D vector.
         * 
         * @param scalar Value to initialize both components to.
         */
        explicit constexpr basic_vec3(T scalar) : x(scalar), y(scalar), z(scalar) {}

        /**
         * @brief Construct a new 3D vector.
         * 
         * @param x X component of the vector.
         * @param y Y component of the vector.
         * @param z Z component of the vector.
         */
        constexpr basic_vec3(T x, T y, T z) : x(x), y(y), z(z) {}

        /** 
         * @brief Get component using an index. 
         * 
         * @remarks This function cannot be constant expression.
         * 
         * @return Component according to index.
         */
        [[nodiscard]] inline T operator[](std::size_t index) const { return (&x)[index]; }

        /**
         * @brief Get component using an index.
         *
         * @note This function cannot be constant expression.
         *
         * @return Component according to index.
         */
        [[nodiscard]] inline T& operator[](std::size_t index) { return (&x)[index]; }

        /**
         * @brief Adds two vectors.
         * 
         * @param vec Vector to add.
         * 
         * @return Sum of the source vectors.
         */
        [[nodiscard]] constexpr basic_vec3<T> operator+(const basic_vec3<T>& vec) const {
            return { x + vec.x, y + vec.y, z + vec.z };
        }

        /**
         * @brief Add a scalar to vector.
         *
         * @param scalar Value to add.
         *
         * @return Sum of the source vector and a scalar value.
         */
        [[nodiscard]] constexpr basic_vec3<T> operator+(T scalar) const {
            return { x + scalar, y + scalar, z + scalar };
        }

        /**
         * @brief Subtracts a vector from a vector.
         *
         * @param vec Vector to subtract.
         *
         * @return Result of the subtraction.
         */
        [[nodiscard]] constexpr basic_vec3<T> operator-(const basic_vec3<T>& vec) const {
            return { x - vec.x, y - vec.y, y - vec.z };
        }

        /**
         * @brief Subtract vector with scalar.
         *
         * @param scalar Value to subtract.
         *
         * @return Result of the subtraction.
         */
        [[nodiscard]] constexpr basic_vec3<T> operator-(T scalar) const {
            return { x - scalar, y - scalar, z - scalar };
        }

        /**
         * @brief Multiplies the components of two vectors by each other.
         *
         * @param vec Vector to multiply.
         *
         * @return Result of the multiplication.
         */
        [[nodiscard]] constexpr basic_vec3<T> operator*(const basic_vec3<T>& vec) const {
            return { x * vec.x, y * vec.y, z * vec.z };
        }

        /**
         * @brief Multiplies a vector by a scalar value.
         *
         * @param scalar Value to multiply.
         *
         * @return Result of the multiplication.
         */
        [[nodiscard]] constexpr basic_vec3<T> operator*(T scalar) const {
            return { x * scalar, y * scalar, z * scalar };
        }

        /**
         * @brief Divides the components of a vector by the components of another vector.
         *
         * @param vec Divisor vector.
         *
         * @return The result of dividing the vectors.
         */
        [[nodiscard]] constexpr basic_vec3<T> operator/(const basic_vec3<T>& vec) const {
            return { x / vec.x, y / vec.y, z / vec.z };
        }

        /**
         * @brief Divides a vector by a scalar value.
         *
         * @param scalar The divisor.
         *
         * @return The source vector divided by a scalar.
         */
        [[nodiscard]] constexpr basic_vec3<T> operator/(T scalar) const {
            return { x / scalar, y / scalar, z / scalar };
        }

        /**
         * @brief Returns a vector pointing in the opposite direction.
         * 
         * @return Vector pointing in the opposite direction.
         */
        [[nodiscard]] constexpr basic_vec3<T> operator-() const {
            return { -x, -y, -z };
        }

        /**
         * @brief Compare two vectors and check that these two vectors are almost equal.
         * 
         * @return True if vectors are almost equal, false otherwise.
         */
        [[nodiscard]] constexpr bool almost_equal(const basic_vec3<T>& vec) const {
            constexpr T epsilon = std::numeric_limits<T>::epsilon();

            return rb::abs(x - vec.x) < epsilon && 
                rb::abs(y - vec.y) < epsilon &&
                rb::abs(z - vec.z) < epsilon;
        }

        /** 
         * @brief Compute dot product with another vector.
         * 
         * @return Dot product between two vectors.
         */
        [[nodiscard]] constexpr T dot(const basic_vec3<T>& vec) const {
            return x * vec.x + y * vec.y + z * vec.z;
        }

        /**
         * @brief Compute squared length of the vector.
         *
         * @return Squared length of the vector.
         */
        [[nodiscard]] constexpr T length_squared() const {
            return dot(*this);
        }

        /**
         * @brief Compute length of the vector.
         *
         * @note This function cannot be a constant expression.
         * 
         * @return Length of the vector.
         */
        [[nodiscard]] T length() const {
            return std::sqrt(length_squared());
        }

        /**
         * @brief Compute normalized vector.
         * 
         * @remarks Normalized vector is a vector with length of one.
         *
         * @note This function cannot be a constant expression.
         *
         * @return Normalized vector.
         */
        [[nodiscard]] basic_vec3<T> normalized() const {
            return (*this) / length();
        }

        /**
         * @brief Compute distance between two points.
         *
         * @note This function cannot be a constant expression.
         * 
         * @param target Target vector.
         * 
         * @return Distance between two points.
         */
        [[nodiscard]] T distance_to(const basic_vec3<T>& vec) const {
            return length(vec - *this)
        }

        /**
         * @brief Compute direction vector to target point.
         *
         * @note This function cannot be a constant expression.
         * 
         * @param target Target vector.
         *
         * @return Direction vector.
         */
        [[nodiscard]] T direction_to(const basic_vec3<T>& target) const {
            return (target - *this).normalized();
        }

        /**
         * @brief Move point towards target point with delta distance.
         *        Will not go past the final value.
         *
         * @note This function cannot be a constant expression.
         *
         * @param target Target vector.
         * @param delta Distance factor.
         *
         * @return New position.
         */
        [[nodiscard]] T move_towards(const basic_vec3<T>& target, T delta) const {
            return *this + direction_to(target) * min(delta, distance_to(target));
        }

        /**
         * @brief Returns a new vector with all components rounded down (towards negative infinity).
         * 
         * @return Vector with all components rounded down.
         */
        [[nodiscard]] basic_vec3<T> floor() const {
            return { std::floor(x), std::floor(y), std::floor(z) };
        }

        /**
         * @brief Returns a new vector with all components rounded to the nearest integer, with halfway cases rounded away from zero.
         *
         * @return Vector with all components rounded.
         */
        [[nodiscard]] basic_vec3<T> round() const {
            return { std::round(x), std::round(y), std::round(z) };
        }

        /**
         * @brief Returns a new vector with all components rounded up (towards positive infinity).
         * 
         * @return Vector with all components rounded up.
         */
        [[nodiscard]] basic_vec3<T> ceil() const {
            return { std::ceil(x), std::ceil(y), std::ceil(z) };
        }

        /**
         * @brief Returns a new vector with all components in absolute values (i.e. positive).
         *
         * @return Vector with all components in absolute values.
         */
        [[nodiscard]] constexpr basic_vec3<T> abs() const {
            return { rb::abs(x), rb::abs(y), rb::abs(z) };
        }

        /**
         * @brief Returns the result of the linear interpolation between this vector and target by amount weight. 
         *        Weight is on the range of 0.0 to 1.0, representing the amount of interpolation.
         * 
         * @return Interpolated vector.
         */
        [[nodiscard]] constexpr basic_vec3<T> lerp(const basic_vec3<T>& target, T weight) const {
            return *this + (target - *this) * weight;
        }

        /** 
         * @brief X component of the vector.
         */
        T x;

        /**
         * @brief Y component of the vector.
         */
        T y;

        /**
         * @brief Z component of the vector.
         */
        T z;
    };

    using vec3 = basic_vec3<float>;
    using ivec3 = basic_vec3<int>;
    using uvec3 = basic_vec3<unsigned int>;
}
