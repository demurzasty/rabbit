#pragma once

#include "fwd.hpp"
#include "../graphics/fwd.hpp"

namespace rb {
    class system {
    public:
        /**
         * @brief Default virtual destructor.
         */
        virtual ~system() = default;

        /**
         * @brief Called once on application startup.
         */
        virtual void initialize(registry& registry);

        /**
         * @brief Called every frame with variable time.
         */
        virtual void update(registry& registry, float elapsed_time);

        /**
         * @brief Called every frame with fixed time.
         */
        virtual void fixed_update(registry& registry, float fixed_time);

        /**
         * @brief Called every frame on render pass.
         */
        virtual void draw(registry& registry, graphics_device& graphics_device);
    };
}
