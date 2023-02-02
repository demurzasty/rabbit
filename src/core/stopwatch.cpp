#include <rabbit/core/stopwatch.hpp>

using namespace rb;

float stopwatch::time() {
    const auto current_time = std::chrono::steady_clock::now();
    const auto elapsed_time = std::chrono::duration_cast<std::chrono::duration<float>>(current_time - m_time);

    return elapsed_time.count();
}

float stopwatch::restart() {
    const auto current_time = std::chrono::steady_clock::now();
    const auto elapsed_time = std::chrono::duration_cast<std::chrono::duration<float>>(current_time - m_time);
    m_time = current_time;

    return elapsed_time.count();
}
