#pragma once 

#include "handle.hpp"

#include <cassert>
#include <type_traits>

namespace rb {
    /**
     * @brief Region-based fixed-size memory allocator. Mainly used for sharing data between CPU and GPU.
     *        Better performance should be achieved with plain-old-data types.
     */
    template<class T>
    class arena {
    public:
        /**
         * @brief Construct a new arena.
         */
        arena() = default;

        /**
         * @brief Disabled copy constructor.
         */
        arena(const arena<T>&) = delete;

        /**
         * @brief Enabled move constructor.
         */
        arena(arena<T>&&) noexcept = default;

        /**
         * @brief Destructor of the arena<T>.
         */
        ~arena() {
            if constexpr (!std::is_standard_layout_v<T> || !std::is_trivial_v<T>) {
                for (std::size_t i = 0, size = m_pool.size(); i < size; ++i) {
                    if (m_pool[i] == handle(i)) {
                        std::destroy_at(std::launder(reinterpret_cast<T*>(&m_data[i])));
                    }
                }
            }
        }

        /**
         * @brief Disabled copy assignment.
         */
        arena<T>& operator=(const arena<T>&) = delete;

        /**
         * @brief Enabled move assignment.
         */
        arena<T>& operator=(arena<T>&&) noexcept = default;

        /**
         * @brief Get data attached to id.
         * 
         * @return Data attached to id.
         */
        const T& operator[](handle id) const {
            assert(valid(id));
            return *std::launder(reinterpret_cast<const T*>(&m_data[std::size_t(id)]));
        }

        /**
         * @brief Get data attached to id.
         *
         * @return Data attached to id.
         */
        T& operator[](handle id) {
            assert(valid(id));
            return *std::launder(reinterpret_cast<T*>(&m_data[std::size_t(id)]));
        }

        /** 
         * @brief Iterates id and data and applies the given function
         *        object to them.
         * 
         * @tparam Func Type of the function object to invoke.
         * @param func A valid function object.
         */
        template<typename Func>
        void each(Func func) {
            for (std::size_t i = 0, size = m_pool.size(); i < size; ++i) {
                if (m_pool[i] == handle(i)) {
                    auto& data = *std::launder(reinterpret_cast<T*>(&m_data[i]));
                    std::invoke(func, m_pool[i], data);
                }
            }
        }

        /**
         * @brief Iterates id and data and applies the given function
         *        object to them.
         *
         * @tparam Func Type of the function object to invoke.
         * @param func A valid function object.
         */
        template<typename Func>
        void each(Func func) const {
            for (std::size_t i = 0, size = m_pool.size(); i < size; ++i) {
                if (m_pool[i] == handle(i)) {
                    const auto& data = *std::launder(reinterpret_cast<const T*>(&m_data[i]));
                    std::invoke(func, m_pool[i], data);
                }
            }
        }

        /**
         * @brief Create and construct new object. 
         */
        template<class... Args>
        handle create(Args&&... args) {
            const auto id = acquire();
            
            if (std::size_t(id) == m_data.size()) {
                m_data.emplace_back();
            }
            
            // For POD types it is not required to run constructor unless arguments are not provided.
            if constexpr ((!std::is_standard_layout_v<T> || !std::is_trivial_v<T>) || sizeof...(Args) > 0) {
                ::new (const_cast<void*>(static_cast<const volatile void*>(&m_data[std::size_t(id)])))
                    T(std::forward<Args>(args)...);
            }

            return id;
        }

        /**
         * @brief Destroy an object.
         * 
         * @warning Changing data of identifier being destroyed can result in undefined behavior.
         */
        void destroy(handle id) {
            assert(valid(id));

            if constexpr (!std::is_standard_layout_v<T> || !std::is_trivial_v<T>) {
                std::destroy_at(std::launder(reinterpret_cast<T*>(&m_data[std::size_t(id)])));
            }

            dispose(id);
        }

        /**
         * @brief Checks if an identifier refers to a valid data.
         * 
         * @return True if id is valid, false otherwise.
         */
        bool valid(handle id) const {
            return std::size_t(id) < m_pool.size() && m_pool[std::size_t(id)] == id;
        }

    private:
        /**
         * @brief Acquire new free id.
         * 
         * @return Free id.
         */
        handle acquire() {
            if (m_disposed == null) {
                return m_pool.emplace_back(handle(m_pool.size()));
            }

            const handle recycled_id = m_disposed;
            m_disposed = m_pool[std::size_t(m_disposed)];
            return m_pool[std::size_t(recycled_id)] = recycled_id;
        }

        /**
         * @brief Dispose id.
         *
         * @param id Id to dispose.
         */
        void dispose(handle id) {
            assert(valid(id));

            m_pool[std::size_t(id)] = m_disposed;
            m_disposed = id;
        }

    private:
        /**
         * @brief ID pool.
         */
        std::vector<handle> m_pool;

        /**
         * @brief Last disposed ID.
         */
        handle m_disposed = null;

        /**
         * @brief Data linear container with explicit creation/destruction.
         */
        std::vector<std::aligned_storage_t<sizeof(T), alignof(T)>> m_data;
    };
}
