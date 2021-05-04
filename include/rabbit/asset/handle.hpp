#pragma once

#include <memory>
#include <future>
#include <chrono>
#include <atomic>
#include <functional>

namespace rb {
    class proxy {
    public:
        proxy(std::shared_future<std::function<std::shared_ptr<void>()>> future)
            : _future(future) {
        }

        bool ready() const {
            return _is_ready || (_is_ready = _future_ready());
        }

        const std::shared_ptr<void>& get() {
            if (!_asset) {
                _asset = _future.get()();
            }
            return _asset;
        }

    private:
        bool _future_ready() const {
            const auto status = _future.wait_for(std::chrono::nanoseconds{ 0 });
            return status == std::future_status::ready ||
                status == std::future_status::deferred;
        }

    private:
        std::shared_future<std::function<std::shared_ptr<void>()>> _future;
        mutable std::shared_ptr<void> _asset;
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
