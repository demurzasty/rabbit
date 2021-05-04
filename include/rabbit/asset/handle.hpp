#pragma once

#include <memory>

namespace rb {
    struct proxy {
        std::shared_ptr<void> asset;
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
            return std::static_pointer_cast<T>(_proxy->asset);
        }

        operator bool() const {
            return _proxy && _proxy->asset;
        }

        std::shared_ptr<T> get() const {
            return std::static_pointer_cast<T>(_proxy->asset);
        }

    private:
        std::shared_ptr<proxy> _proxy;
    };
}
