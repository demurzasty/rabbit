#pragma once 

#include "arena.hpp"
#include "config.hpp"
#include "memory.hpp"
#include "type_traits.hpp"

#include <limits>
#include <vector>

namespace rb {
    struct region {
        std::size_t offset = 0;
        std::size_t size = 0;
    };

    /**
     * @brief Region-based variable-size memory management tool.
     *        No data will be allocated, only region will be computed.
     *        Mainly used for foreign memory allocation, i.e. for GPU.
     */
    class zone {
    public:
        id_type assign(std::size_t p_size) {
            const auto id = m_regions.create();

            auto& region = m_regions[id];
            region.offset = m_offset;
            region.offset = m_offset + p_size;

            m_offset += p_size;
            return id;
        }

        void free(id_type p_id) {
            assert(valid(p_id));

            m_regions.destroy(p_id);
        }

        bool valid(id_type p_id) const {
            return m_regions.valid(p_id);
        }

    private:
        arena<region> m_regions;
        std::size_t m_offset = 0;
    };
}
