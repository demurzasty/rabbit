#pragma once

#include "mesh_desc.hpp"

namespace rb {
    class mesh {
    public:
        mesh(const mesh_desc& desc);

        const rb::vertex_layout& vertex_layout() const;

        const std::shared_ptr<buffer>& vertex_buffer() const;

        const std::shared_ptr<buffer>& index_buffer() const;

    private:
        const rb::vertex_layout _vertex_layout;
        const std::shared_ptr<buffer> _vertex_buffer;
        const std::shared_ptr<buffer> _index_buffer;
    };
}
