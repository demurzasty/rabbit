#pragma once 

#include "vec2.hpp"

namespace rb {
    template<typename T>
    struct basic_rect {
        /**
         * @brief Default rectangle constructor. 
         *        No initialization to improve performance.
         */
        basic_rect() = default;

        /**
         * @brief Construct a new rectangle.
         * 
         * @param x Left of the rectangle.
         * @param y Top of the rectangle.
         * @param width Width of the rectangle.
         * @param height Height of the rectangle.
         */
        constexpr basic_rect(T x, T y, T width, T height) noexcept : position(x, y), size(width, height) {}

        /**
         * @brief Construct a new rectangle.
         *
         * @param position Beginning corner.
         * @param size Size from position to end.
         */
        constexpr basic_rect(const basic_vec2<T>& position, const basic_vec2<T>& size) noexcept : position(position), size(size) {}

        /**
         * @brief Check if point is inside a rectangle.
         *
         * @return True if rectangle contains point, false otherwise.
         */
        bool contains(const basic_vec2<T>& point) const {
            return point.x >= position.x && point.x < position.x + size.x &&
                point.y >= position.y && point.y < position.y + size.y;
        }

        /**
         * @brief Ending corner. This is calculated as position + size.
         */
        constexpr basic_vec2<T> end() const noexcept {
            return position + size;
        }

        /**
         * @brief Beginning corner. Typically has values lower than end.
         */
        basic_vec2<T> position;
        
        /**
         * @brief Size from position to end. Typically, all components are positive.
         */
        basic_vec2<T> size;
    };

    using rect = basic_rect<float>;
    using irect = basic_rect<int>;
    using urect = basic_rect<unsigned int>;
}
