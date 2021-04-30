#include <rabbit/asset/asset_manager.hpp>

#include <fstream>

using namespace rb;

asset_manager::asset_manager(injector& injector)
    : _injector(injector) {
}

std::shared_ptr<void> asset_manager::_load(id_type asset_id, const std::string& filename) {
    auto& asset = _assets[filename];
    if (!asset.expired()) {
        return asset.lock();
    }

    auto& loader = _loaders.at(asset_id);
    auto loaded_asset = loader->load(filename, _load_metadata(filename));
    asset = loaded_asset;
    return loaded_asset;
}

json asset_manager::_load_metadata(const std::string& filename) const {
    json metadata;

    std::ifstream stream{ filename + ".meta", std::ios::in };
    if (stream.is_open()) {
        stream >> metadata;
    }

    return metadata;
}
