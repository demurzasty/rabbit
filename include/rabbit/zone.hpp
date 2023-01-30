#pragma once 

#include "math.hpp"
#include "arena.hpp"
#include "config.hpp"
#include "memory.hpp"
#include "type_traits.hpp"

#include <limits>
#include <vector>

namespace rb {
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

        region& operator[](id_type p_id) {
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
