#pragma once

#include "../core/entity.hpp"

namespace rb {
    class renderer {
    public:
        /**
         * @brief Default virtual destructor.
         */
        virtual ~renderer() = default;

        /**
         * @brief Render whole scene.
         */
        virtual void render(registry& registry) = 0;
    };
}
