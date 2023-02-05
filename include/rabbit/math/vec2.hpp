#pragma once 

#include "math.hpp"

namespace rb {
    /**
     * @brief Representation of 2D vectors and points.
     */
    template<typename T>
    struct basic_vec2 {
        /**
         * @brief Make 2D vector with both of its components set to zero.
         *
         * @return 2D vector with both of its components set to zero.
         */
        [[nodiscard]] static constexpr basic_vec2<T> zero() { return { 0, 0 }; }

        /**
         * @brief Make 2D vector with both of its components set to one.
         * 
         * @return 2D vector with both of its components set to one.
         */
        [[nodiscard]] static constexpr basic_vec2<T> one() { return { 1, 1 }; }

        /**
         * @brief Make 2D vector with X component set to one.
         *
         * @return 2D vector with X component set to one.
         */
        [[nodiscard]] static constexpr basic_vec2<T> axis_x() { return { 1, 0 }; }

        /**
         * @brief Make 2D vector with Y component set to one.
         *
         * @return 2D vector with Y component set to one.
         */
        [[nodiscard]] static constexpr basic_vec2<T> axis_y() { return { 0, 1 }; }

        /**
         * @brief Construct a new uninitialized 2D vector.
         */
        basic_vec2() = default;

        /**
         * @brief Construct a new 2D vector.
         * 
         * @param scalar Value to initialize both components to.
         */
        explicit constexpr basic_vec2(T scalar) : x(scalar), y(scalar) {}

        /**
         * @brief Construct a new 2D vector.
         * 
         * @param x X component of the vector.
         * @param y Y component of the vector.
         */
        constexpr basic_vec2(T x, T y) : x(x), y(y) {}

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
        [[nodiscard]] constexpr basic_vec2<T> operator+(const basic_vec2<T>& vec) const {
            return { x + vec.x, y + vec.y };
        }

        /**
         * @brief Add a scalar to vector.
         *
         * @param scalar Value to add.
         *
         * @return Sum of the source vector and a scalar value.
         */
        [[nodiscard]] constexpr basic_vec2<T> operator+(T scalar) const {
            return { x + scalar, y + scalar };
        }

        /**
         * @brief Subtracts a vector from a vector.
         *
         * @param vec Vector to subtract.
         *
         * @return Result of the subtraction.
         */
        [[nodiscard]] constexpr basic_vec2<T> operator-(const basic_vec2<T>& vec) const {
            return { x - vec.x, y - vec.y };
        }

        /**
         * @brief Subtract vector with scalar.
         *
         * @param scalar Value to subtract.
         *
         * @return Result of the subtraction.
         */
        [[nodiscard]] constexpr basic_vec2<T> operator-(T scalar) const {
            return { x - scalar, y - scalar };
        }

        /**
         * @brief Multiplies the components of two vectors by each other.
         *
         * @param vec Vector to multiply.
         *
         * @return Result of the multiplication.
         */
        [[nodiscard]] constexpr basic_vec2<T> operator*(const basic_vec2<T>& vec) const {
            return { x * vec.x, y * vec.y };
        }

        /**
         * @brief Multiplies a vector by a scalar value.
         *
         * @param scalar Value to multiply.
         *
         * @return Result of the multiplication.
         */
        [[nodiscard]] constexpr basic_vec2<T> operator*(T scalar) const {
            return { x * scalar, y * scalar };
        }

        /**
         * @brief Divides the components of a vector by the components of another vector.
         *
         * @param vec Divisor vector.
         *
         * @return The result of dividing the vectors.
         */
        [[nodiscard]] constexpr basic_vec2<T> operator/(const basic_vec2<T>& vec) const {
            return { x / vec.x, y / vec.y };
        }

        /**
         * @brief Divides a vector by a scalar value.
         *
         * @param scalar The divisor.
         *
         * @return The source vector divided by a scalar.
         */
        [[nodiscard]] constexpr basic_vec2<T> operator/(T scalar) const {
            return { x / scalar, y / scalar };
        }

        /**
         * @brief Returns a vector pointing in the opposite direction.
         * 
         * @return Vector pointing in the opposite direction.
         */
        [[nodiscard]] constexpr basic_vec2<T> operator-() const {
            return { -x, -y };
        }

        /**
         * @brief Compare two vectors and check that these two vectors are almost equal.
         * 
         * @return True if vectors are almost equal, false otherwise.
         */
        [[nodiscard]] constexpr bool almost_equal(const basic_vec2<T>& vec) const {
            constexpr T epsilon = std::numeric_limits<float>::epsilon();

            return std::abs(x - vec.x) < epsilon && std::abs(y - vec.y) < epsilon;
        }

        /** 
         * @brief Compute dot product with another vector.
         * 
         * @return Dot product between two vectors.
         */
        [[nodiscard]] constexpr T dot(const basic_vec2<T>& vec) const {
            return x * vec.x + y * vec.y;
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
        [[nodiscard]] basic_vec2<T> normalized() const {
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
        [[nodiscard]] T distance_to(const basic_vec2<T>& vec) const {
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
        [[nodiscard]] T direction_to(const basic_vec2<T>& target) const {
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
        [[nodiscard]] T move_towards(const basic_vec2<T>& target, T delta) const {
            return *this + direction_to(target) * min(delta, distance_to(target));
        }

        /**
         * @brief Rotate a point around another point.
         *
         * @param pivot The point around which we will rotate another point.
         * @param angle Rotation angle in radians.
         *
         * @return Rotated point around a pivot.
         */
        template<typename T>
        [[nodiscard]] basic_vec2<T> rotated(const basic_vec2<T>& pivot, T angle) const {
            // Compute sinus and cosinus from angle in radians.
            T s = std::sin(angle);
            T c = std::cos(angle);

            // Translate point back to origin.
            basic_vec2<T> origin(x - pivot.x, y - pivot.y);

            // Rotate point around a pivot.
            basic_vec2<T> offset(origin.x* c - origin.y * s, origin.x* s + origin.y * c);

            // Translate point back.
            return { offset.x + pivot.x, offset.y + pivot.y };
        }

        /**
         * @brief Returns a new vector with all components rounded down (towards negative infinity).
         * 
         * @return Vector with all components rounded down.
         */
        [[nodiscard]] basic_vec2<T> floor() const {
            return { std::floor(x), std::floor(y) };
        }

        /**
         * @brief Returns a new vector with all components rounded to the nearest integer, with halfway cases rounded away from zero.
         *
         * @return Vector with all components rounded.
         */
        [[nodiscard]] basic_vec2<T> round() const {
            return { std::round(x), std::round(y) };
        }

        /**
         * @brief Returns a new vector with all components rounded up (towards positive infinity).
         * 
         * @return Vector with all components rounded up.
         */
        [[nodiscard]] basic_vec2<T> ceil() const {
            return { std::ceil(x), std::ceil(y) };
        }

        /**
         * @brief Returns a new vector with all components in absolute values (i.e. positive).
         *
         * @return Vector with all components in absolute values.
         */
        [[nodiscard]] constexpr basic_vec2<T> abs() const {
            return { rb::abs(x), rb::abs(y) };
        }

        /**
         * @brief Returns the aspect ratio of this vector, the ratio of x to y.
         *
         * @return Aspect ratio.
         */
        [[nodiscard]] constexpr T aspect() const {
            return x / y;
        }

        /**
         * @brief Returns the result of the linear interpolation between this vector and target by amount weight. 
         *        Weight is on the range of 0.0 to 1.0, representing the amount of interpolation.
         * 
         * @return Interpolated vector.
         */
        [[nodiscard]] constexpr basic_vec2<T> lerp(const basic_vec2<T>& target, T weight) const {
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
    };

    using vec2 = basic_vec2<float>;
    using ivec2 = basic_vec2<int>;
    using uvec2 = basic_vec2<unsigned int>;
}
