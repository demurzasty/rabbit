#pragma once 

#include "pool.hpp"
#include "config.hpp"
#include "memory.hpp"
#include "type_traits.hpp"

#include <vector>
#include <limits>
#include <cassert>

namespace rb {
    /**
     * @brief Region-based fixed-size memory allocator. Mainly used for sharing data between CPU and GPU.
     *        Better performance should be achieved with plain-old-data types.
     */
    template<class T>
    class arena {
    public:
        arena() = default;

        arena(const arena<T>&) = delete;

        arena(arena<T>&&) noexcept = default;

        ~arena() {
            if constexpr (!is_pod_v<T>) {
                m_pool.each([this](id_type p_id) {
                    std::destroy_at(std::launder(reinterpret_cast<T*>(&m_data[std::size_t(p_id)])));
                });
            }
        }

        arena<T>& operator=(const arena<T>&) = delete;

        arena<T>& operator=(arena<T>&&) noexcept = default;

        const T& operator[](id_type p_id) const {
            assert(valid(p_id));
            return *std::launder(reinterpret_cast<const T*>(&m_data[std::size_t(p_id)]));
        }

        T& operator[](id_type p_id) {
            assert(valid(p_id));
            return *std::launder(reinterpret_cast<T*>(&m_data[std::size_t(p_id)]));
        }

        template<typename Func>
        void each(Func p_func) {
            m_pool.each([&p_func](id_type p_id) {
                auto& data = *std::launder(reinterpret_cast<T*>(&m_data[i]));
                std::invoke(p_func, p_id, data);
            });
        }

        template<typename Func>
        void each(Func p_func) const {
            m_pool.each([&p_func](id_type p_id) {
                const auto& data = *std::launder(reinterpret_cast<const T*>(&m_data[i]));
                std::invoke(p_func, p_id, data);
            });
        }

        /**
         * @brief Create and construct new object. 
         */
        template<class... Args>
        id_type create(Args&&... p_args) {
            const auto id = m_pool.acquire();
            
            if (std::size_t(id) == m_data.size()) {
                m_data.emplace_back();
            }
            
            // For POD types it is not required to run constructor unless arguments are not provided.
            if constexpr (!is_pod_v<T> || sizeof...(Args) > 0) {
                construct_at(reinterpret_cast<T*>(&m_data[std::size_t(id)]), std::forward<Args>(p_args)...);
            }

            return id;
        }

        /**
         * @brief Destroy an object.
         * 
         * @warning Changing data of identifier being destroyed can result in undefined behavior.
         */
        void destroy(id_type p_id) {
            assert(valid(p_id));

            if constexpr (!is_pod_v<T>) {
                std::destroy_at(std::launder(reinterpret_cast<T*>(&m_data[std::size_t(p_id)])));
            }

            m_pool.dispose(p_id);
        }

        /**
         * @brief Checks if an identifier refers to a valid data.
         */
        bool valid(id_type p_id) const {
            return m_pool.valid(p_id);
        }

    private:
        pool m_pool;
        std::vector<std::aligned_storage_t<sizeof(T), alignof(T)>> m_data;
    };
}
