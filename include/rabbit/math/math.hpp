#pragma once

#include <cmath>
#include <type_traits>

namespace rb {
    template<typename T>
    constexpr T pi() noexcept {
        return T{ 3.1415926535897931L };
    }

    template<typename T>
    constexpr T rad2deg(T rad) noexcept {
        return rad * 180 / pi<T>();
    }

    template<typename T>
    constexpr T deg2rad(T deg) noexcept {
        return deg * pi<T>() / 180;
    }

    template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
    constexpr T log2(T index) noexcept {
        T result{ 0 };
        while (index >>= 1) {
            ++result;
        }
        return result;
    }

    template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    inline T log2(T index) noexcept {
        return std::log2(index);
    }
}
