#pragma once 

#include "config.hpp"
#include "memory.hpp"
#include "type_traits.hpp"

#include <vector>
#include <limits>
#include <cassert>

namespace rb {
    class pool {
    public:
        pool() = default;

        pool(const pool&) = delete;

        pool(pool&&) noexcept = default;

        pool& operator=(const pool&) = delete;

        pool& operator=(pool&&) noexcept = default;

        template<typename Func>
        void each(Func p_func) {
            for (std::size_t i = 0, size = m_pool.size(); i < size; ++i) {
                if (m_pool[i] == id_type(i)) {
                    std::invoke(p_func, m_pool[i]);
                }
            }
        }

        template<typename Func>
        void each(Func p_func) const {
            for (std::size_t i = 0, size = m_pool.size(); i < size; ++i) {
                if (m_pool[i] == id_type(i)) {
                    std::invoke(p_func, m_pool[i]);
                }
            }
        }

        id_type acquire() {
            if (m_disposed == null) {
                return m_pool.emplace_back(id_type(m_pool.size()));
            }

            const auto recycled_id = m_disposed;
            m_disposed = m_pool[std::size_t(m_disposed)];
            return m_pool[std::size_t(recycled_id)] = recycled_id;
        }

        void dispose(id_type p_id) {
            assert(valid(p_id));

            m_pool[std::size_t(p_id)] = m_disposed;
            m_disposed = p_id;
        }

        /**
         * @brief Checks if an identifier refers to a valid data.
         */
        bool valid(id_type p_id) const {
            return std::size_t(p_id) < m_pool.size() && m_pool[std::size_t(p_id)] == p_id;
        }

    private:
        std::vector<id_type> m_pool;
        id_type m_disposed = null;
    };
}
