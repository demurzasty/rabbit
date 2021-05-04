
#pragma once

#include "loader.hpp"
#include "handle.hpp"
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
        handle<Asset> load(std::launch launch, const std::string& filename) {
            return { _load(launch, type_id<Asset>().hash(), filename) };
        }

        template<typename Asset>
        handle<Asset> load(const std::string& filename) {
            return load<Asset>(std::launch::async, filename);
        }

    private:
        std::shared_ptr<proxy> _load(std::launch launch, id_type asset_id, const std::string& filename);

        json _load_metadata(const std::string& filename) const;

    private:
        injector& _injector;
        std::unordered_map<id_type, std::shared_ptr<loader>> _loaders;
        std::unordered_map<std::string, std::weak_ptr<proxy>> _assets;
    };
}
