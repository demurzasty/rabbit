#pragma once 

#include "math.hpp"
#include "config.hpp"
#include "type_traits.hpp"

#include <memory>
#include <vector>
#include <limits>
#include <cassert>

namespace rb {
    /**
     * Creates a T object initialized with arguments p_args... at given address p_ptr.
     * 
     * @note Waiting for C++20.
     */
    template<class T, class... Args>
    constexpr T* construct_at(T* p_ptr, Args&&... p_args) {
        return ::new (const_cast<void*>(static_cast<const volatile void*>(p_ptr))) 
            T(std::forward<Args>(p_args)...);
    }

    template<class IdType = id_type>
    class pool {
    public:
        using id_type = IdType;

    public:
        pool() = default;

        pool(const pool<IdType>&) = delete;

        pool(pool<IdType>&&) noexcept = default;

        pool<IdType>& operator=(const pool<IdType>&) = delete;

        pool<IdType>& operator=(pool<IdType>&&) noexcept = default;

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
    
    /**
     * @brief Region-based fixed-size memory allocator. Mainly used for sharing data between CPU and GPU.
     *        Better performance should be achieved with plain-old-data types.
     */
    template<class T, class IdType = id_type>
    class arena {
    public:
        using id_type = IdType;

    public:
        arena() = default;

        arena(const arena<T, IdType>&) = delete;

        arena(arena<T, IdType>&&) noexcept = default;

        ~arena() {
            if constexpr (!is_pod_v<T>) {
                m_pool.each([this](id_type p_id) {
                    std::destroy_at(std::launder(reinterpret_cast<T*>(&m_data[std::size_t(p_id)])));
                });
            }
        }

        arena<T, IdType>& operator=(const arena<T, IdType>&) = delete;

        arena<T, IdType>& operator=(arena<T, IdType>&&) noexcept = default;

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
            m_pool.each([this, &p_func](id_type p_id) {
                auto& data = *std::launder(reinterpret_cast<T*>(&m_data[std::size_t(p_id)]));
                std::invoke(p_func, p_id, data);
            });
        }

        template<typename Func>
        void each(Func p_func) const {
            m_pool.each([&p_func](id_type p_id) {
                const auto& data = *std::launder(reinterpret_cast<const T*>(&m_data[std::size_t(p_id)]));
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
        pool<id_type> m_pool;
        std::vector<std::aligned_storage_t<sizeof(T), alignof(T)>> m_data;
    };
    
    struct region {
        std::size_t offset;
        std::size_t size;
        std::size_t capacity;
    };

    /**
     * @brief Region-based variable-size memory management tool.
     *        No data will be allocated, only region will be computed.
     *        Mainly used for foreign memory allocation, i.e. for GPU.
     */
    class zone {
    public:
        const region& operator[](id_type p_id) const {
            assert(valid(p_id));
            return m_regions[p_id];
        }

        const region& operator[](id_type p_id) {
            assert(valid(p_id));
            return m_regions[p_id];
        }

        id_type create(std::size_t p_capacity) {
            const auto id = m_regions.create();

            auto& region = m_regions[id];
            region.offset = m_offset;
            region.size = 0;
            region.capacity = next_power_of_two(p_capacity);

            m_offset += region.capacity;
            return id;
        }

        const region& assign(id_type p_id, std::size_t p_size) {
            assert(valid(p_id));

            auto& region = m_regions[p_id];
            if (p_size > region.capacity) {
                if (m_offset != region.offset + region.capacity) {
                    region.offset = m_offset;
                }

                region.capacity = next_power_of_two(p_size);
                m_offset += region.capacity;
            } 

            region.size = p_size;
            return region;
        }

        void destroy(id_type p_id) {
            assert(valid(p_id));

            m_regions.destroy(p_id);
        }

        /**
         * @brief Checks if an identifier refers to a valid data.
         */
        bool valid(id_type p_id) const {
            return m_regions.valid(p_id);
        }

    private:
        arena<region> m_regions;
        std::size_t m_offset = 0;
    };
}
