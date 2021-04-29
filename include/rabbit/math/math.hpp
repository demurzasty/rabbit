#pragma once

namespace rb {
    template<typename T>
    constexpr T pi() noexcept {
        return T{ 3.1415926535897931L };
    }

    template<typename T>
    constexpr T rad2deg(T rad) {
        return rad * 180 / pi<T>();
    }

    template<typename T>
    constexpr T deg2rad(T deg) {
        return deg * pi<T>() / 180;
    }
}
