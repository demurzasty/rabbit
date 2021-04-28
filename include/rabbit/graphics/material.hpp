#pragma once

#include "fwd.hpp"
#include "../math/vec3.hpp"

#include <memory>

namespace rb {
    class material {
    public:
        material(const material_desc& desc);

    private:
        vec3f _base_color;
        float _roughness;
        float _metallic;
        const std::shared_ptr<texture> _albedo_map;
        const std::shared_ptr<texture> _normal_map;
        const std::shared_ptr<texture> _roughness_map;
        const std::shared_ptr<texture> _metallic_map;
        const std::shared_ptr<texture> _emissive_map;
    };
}
