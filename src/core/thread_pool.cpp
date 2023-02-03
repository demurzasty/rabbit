#include <rabbit/core/thread_pool.hpp>

using namespace rb;

thread_pool::thread_pool(unsigned int thread_count)
    : m_threads(thread_count) {
    for (std::thread& thread : m_threads) {
        thread = std::thread(&thread_pool::worker, this);
    }
}

thread_pool::~thread_pool() {
    m_running = false;

    m_available.notify_all();

    for (std::thread& thread : m_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void thread_pool::worker() {
    while (m_running) {
        std::unique_lock lock(m_mutex);

        m_available.wait(lock, [this] { return !m_tasks.empty() || !m_running; });

        if (m_running) {
            std::function<void()> task = std::move(m_tasks.front());
            m_tasks.pop();

            lock.unlock();

            std::invoke(task);
        }
    }
}
