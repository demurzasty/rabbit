#pragma once

#include "fwd.hpp"
#include "../math/vec3.hpp"

#include <memory>

namespace rb {
    /**
     * @brief Material descriptor.
     */
    struct material_desc {
        /**
         * @brief Base Color defines the overall color of the Material, taking in a Vector3 (RGB)
         *        value where each channel is automatically clamped between 0 and 1.
         */
        vec3f base_color{ 1.0f, 1.0f, 1.0f };

        /**
         * @brief The Roughness input controls how rough or smooth a Material's surface is.
         *
         *        Rough Materials scatter reflected light in more directions than smooth Materials,
         *        which controls how blurry or sharp a reflection is
         *        (or how broad or tight a specular highlight is).
         *
         *        A Roughness of 0 (smooth) results in a mirror reflection
         *        and roughness of 1 (rough) results in a diffuse (or matte) surface.
         */
        float roughness{ 0.8f };

        /**
         * @brief The Metallic input controls how 'metal-like' your surface will be.
         *
         *        Nonmetals have Metallic values of 0 and metals have Metallic values of 1.
         *        For pure surfaces, such as pure metal, pure stone, pure plastic, etc.
         *        this value will be 0 or 1, not anything in between.
         *
         *        When creating hybrid surfaces like corroded, dusty, or rusty metals,
         *        you may find that you need some value between 0 and 1.
         */
        float metallic{ 0.0f };

        std::shared_ptr<texture> albedo_map;
        std::shared_ptr<texture> normal_map;
        std::shared_ptr<texture> roughness_map;
        std::shared_ptr<texture> metallic_map;
        std::shared_ptr<texture> emissive_map;
    };
}
