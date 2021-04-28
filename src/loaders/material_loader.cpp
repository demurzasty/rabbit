#include <rabbit/loaders/material_loader.hpp>
#include <rabbit/asset/asset_manager.hpp>
#include <rabbit/core/config.hpp>
#include <rabbit/graphics/material.hpp>
#include <rabbit/graphics/material_desc.hpp>
#include <rabbit/core/json.hpp>

#include <array>
#include <fstream>

using namespace rb;

material_loader::material_loader(asset_manager& asset_manager)
    : _asset_manager(asset_manager) {
}

std::shared_ptr<void> material_loader::load(const std::string& filename, const json& metadata) {
    std::ifstream stream{ filename, std::ios::in };
    RB_ASSERT(stream.is_open(), "Cannot open file");

    json json;
    stream >> json;

    stream.close();

    material_desc desc;

    if (json.contains("base_color")) {
        auto& base_color = json["base_color"];
        desc.base_color = { base_color[0], base_color[1], base_color[2] };
    }

    if (json.contains("roughness")) {
        desc.roughness = json["roughness"];
    }

    if (json.contains("metallic")) {
        desc.metallic = json["metallic"];
    }

    if (json.contains("albedo_map")) {
        desc.albedo_map = _asset_manager.load<texture>(json["albedo_map"]);
    }

    if (json.contains("normal_map")) {
        desc.normal_map = _asset_manager.load<texture>(json["normal_map"]);
    }

    if (json.contains("roughness_map")) {
        desc.roughness_map = _asset_manager.load<texture>(json["roughness_map"]);
    }

    if (json.contains("metallic_map")) {
        desc.metallic_map = _asset_manager.load<texture>(json["metallic_map"]);
    }

    if (json.contains("emissive_map")) {
        desc.emissive_map = _asset_manager.load<texture>(json["emissive_map"]);
    }

    return std::make_shared<material>(desc);
}
