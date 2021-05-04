#pragma once

#include <memory>
#include <future>
#include <chrono>
#include <atomic>

namespace rb {
    class proxy {
    public:
        proxy(std::shared_future<std::shared_ptr<void>> future)
            : _future(future) {
        }

        bool ready() const {
            return _is_ready || (_is_ready = _ready());
        }

        const std::shared_ptr<void>& get() {
            return _future.get();
        }

    private:
        bool _ready() const {
            const auto status = _future.wait_for(std::chrono::nanoseconds{ 0 });
            return status == std::future_status::ready ||
                status == std::future_status::deferred;
        }

    private:
        std::shared_future<std::shared_ptr<void>> _future;
        mutable std::atomic<bool> _is_ready{ false };
    };

    /**
     * @brief Asset handle.
     *                                    texture
     *                                  /
     *                                /
     *        handle<texture> - proxy - - texture
     *                                \
     *                                  \
     *                                    texture
     */
    template<typename T>
    class handle {
    public:
        handle() = default;

        handle(const std::shared_ptr<proxy>& proxy)
            : _proxy(proxy) {
        }

        handle(const handle<T>&) = default;
        handle(handle<T>&&) = default;

        handle<T>& operator=(const handle<T>&) = default;
        handle<T>& operator=(handle&&) = default;

        T* operator->() const {
            return std::static_pointer_cast<T>(_proxy->get()).get();
        }

        operator bool() const {
            return _proxy.operator bool();
        }

        bool ready() const {
            return _proxy && _proxy->ready();
        }

        std::shared_ptr<T> get() const {
            return std::static_pointer_cast<T>(_proxy->get());
        }

    private:
        std::shared_ptr<proxy> _proxy;
    };
}
