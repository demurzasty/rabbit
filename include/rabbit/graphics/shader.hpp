#pragma once

#include "fwd.hpp"
#include "shader_desc.hpp"

#include <vector>
#include <optional>

namespace rb {
    /**
     * @brief Shader interface.
     */
    class shader {
    public:
        /**
         * @brief Default virtual destructor.
         */
        virtual ~shader() = default;

        /**
         * @brief Returns vertex layout descriptor.
         */
        const vertex_layout& vertex_layout() const;

        std::optional<shader_binding_desc> binding(std::size_t slot) const;

        const std::vector<shader_binding_desc>& bindings() const;

    protected:
        shader(const shader_desc& desc);

    private:
        const rb::vertex_layout _vertex_layout;
        const std::vector<shader_binding_desc> _bindings;
    };
}