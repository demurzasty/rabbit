#pragma once 

#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <memory>
#include <atomic>
#include <future>
#include <functional>
#include <type_traits>

namespace rb {
    /**
     * @brief Thread pool.
     */
    class thread_pool {
    public:
        /**
         * @brief Construct a new thread pool.
         *
         * @param thread_count Thread count to create.
         */
        thread_pool(unsigned int thead_count = std::thread::hardware_concurrency());

        /**
         * @brief Disabled copy constructor.
         */
        thread_pool(const thread_pool&) = delete;

        /**
         * @brief Enabled move constructor.
         */
        thread_pool(thread_pool&&) noexcept = default;

        /**
         * @brief Disabled copy assignment.
         */
        thread_pool& operator=(const thread_pool&) = delete;

        /**
         * @brief Enabled move assignment.
         */
        thread_pool& operator=(thread_pool&&) noexcept = default;

        /**
         * @brief Destruct the thread pool.
         */
        ~thread_pool();

        /**
         * @brief Submit a function with zero or more arguments into the task queue.
         * 
         * @tparam Func The type of the function.
         * @tparam Args The types of arguments to pass to the function.
         * @tparam Ret The return type of the function.
         * 
         * @param func The function to submit.
         * @param args The arguments to pass to the function.
         * 
         * @return A future to be used later to wait for the function to finish and obtain its returned value.
         */
        template<typename Func, typename... Args, typename Ret = std::invoke_result_t<std::decay_t<Func>, std::decay_t<Args>...>>
        [[nodiscard]] std::future<Ret> submit(Func&& func, Args&&... args) {
            std::function<Ret()> task = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
            std::shared_ptr<std::promise<Ret>> promise = std::make_shared<std::promise<Ret>>();

            {
                std::unique_lock lock(m_mutex);
                m_tasks.push([task, promise] {
                    if constexpr (std::is_void_v<Ret>) {
                        std::invoke(task);
                        promise->set_value();
                    } else {
                        promise->set_value(std::invoke(task));
                    }
                });
            }

            m_available.notify_one();
            return promise->get_future();
        }

    private:
        /**
         * @brief A worker function to be assigned to each thread in the pool.
         */
        void worker();

        /**
         * @brief An atomic variable indicating to the workers to keep running. 
         */
        std::atomic<bool> m_running = true;

        /**
         * @brief  A condition variable used to notify worker() that a new task has become available.
         */
        std::condition_variable m_available;

        /**
         * @brief A mutex to synchronize access to the task queue by different threads.
         */
        mutable std::mutex m_mutex;

        /**
         * @brief A queue of tasks to be executed by the threads.
         */
        std::queue<std::function<void()>> m_tasks;

        /**
         * @brief Active workers container.
         */
        std::vector<std::thread> m_threads;
    };
}
