
#pragma once

#include "loader.hpp"
#include "../core/non_copyable.hpp"
#include "../core/injector.hpp"
#include "../core/type_info.hpp"

#include <string>
#include <memory>
#include <unordered_map>

// TODO: Identify asset by unique id.

namespace rb {
    class asset_manager : public non_copyable {
    public:
        asset_manager(injector& injector);

        template<typename Asset, typename Loader>
        void add_loader() {
            _loaders.emplace(type_id<Asset>().hash(), _injector.resolve<Loader>());
        }

        template<typename Asset>
        std::shared_ptr<Asset> load(const std::string& filename) {
            return std::static_pointer_cast<Asset>(_load(type_id<Asset>().hash(), filename));
        }

    private:
        std::shared_ptr<void> _load(id_type asset_id, const std::string& filename);

        json _load_metadata(const std::string& filename) const;

    private:
        injector& _injector;
        std::unordered_map<id_type, std::shared_ptr<loader>> _loaders;
        std::unordered_map<std::string, std::weak_ptr<void>> _assets;
    };
}
