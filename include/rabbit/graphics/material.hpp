#pragma once

#include "fwd.hpp"
#include "../math/vec3.hpp"

#include <memory>

namespace rb {
    class material {
    public:
        material(const material_desc& desc);

        const vec3f& base_color() const;

        float roughness() const;

        float metallic() const;

        const std::shared_ptr<texture>& albedo_map() const;

        const std::shared_ptr<texture>& normal_map() const;

        const std::shared_ptr<texture>& roughness_map() const;

        const std::shared_ptr<texture>& metallic_map() const;

        const std::shared_ptr<texture>& emissive_map() const;

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
