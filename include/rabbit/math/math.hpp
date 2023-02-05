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

    /**
     * @brief Returns smaller value between two values.
     *
     * @return Smaller value.
     */
    template<typename T>
    [[nodiscard]] constexpr T min(const T& a, const T& b) noexcept {
        return (a < b) ? a : b;
    }

    /**
     * @brief Returns greater value between two values.
     * 
     * @return Greater value.
     */
    template<typename T>
    [[nodiscard]] constexpr T max(const T& a, const T& b) noexcept {
        return (a > b) ? a : b;
    }

    /**
     * @brief Make absolute function constexpr.
     * 
     * @return Absolute value.
     */
    template<typename T>
    [[nodiscard]] constexpr T abs(const T& a) noexcept {
        return a < T(0) ? -a : a;
    }

    constexpr float pi_v = basic_pi<float>;
}
