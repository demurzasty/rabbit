#pragma once 

#include <cmath>
#include <limits>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif 

namespace rb {
    template<typename T>
    constexpr T basic_pi = T(3.14159265358979323846);

    template<typename T>
    [[nodiscard]] constexpr T min(const T& a, const T& b) noexcept {
        return (a < b) ? a : b;
    }

    template<typename T>
    [[nodiscard]] constexpr T max(const T& a, const T& b) noexcept {
        return (a > b) ? a : b;
    }

    constexpr float pi_v = basic_pi<float>;
}
