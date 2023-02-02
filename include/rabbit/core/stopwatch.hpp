#pragma once 

#include <chrono>

namespace rb {
    /**
     * @brief Utility class that measures the elapsed time.
     */
    class stopwatch {
    public:
        /**
         * @brief Get the elapsed time.
         * 
         * @return Time elapsed in seconds.
         */
        float time();

        /**
         * @brief Restart the stopwatch.
         * 
         * @return Time elapsed in seconds.
         */
        float restart();

    private:
        /**
         * @brief Store last time.
         */
        std::chrono::time_point<std::chrono::steady_clock> m_time = std::chrono::steady_clock::now();
    };
}
