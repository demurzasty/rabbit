#pragma once 

#include <chrono>

namespace rb {
    template<typename T = float, typename Period = std::ratio<1>, typename Clock = std::chrono::high_resolution_clock>
    class stopwatch {
    public:
        using data_type = T;
        using duration_type = std::chrono::duration<T>;
        using period_type = Period;
        using clock_type = Clock;

    public:
        T reset() {
            const auto current = Clock::now();
            const auto duration = std::chrono::duration_cast<duration_type>(current - m_time);
            m_time = current;

            return duration.count();
        }

    private:
        std::chrono::time_point<clock_type, duration_type> m_time = Clock::now();
    };
}
