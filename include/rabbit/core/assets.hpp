#pragma once 

#include "reference.hpp"
#include "type_info.hpp"

#include <map>
#include <string>
#include <memory>
#include <functional>
#include <string_view>

namespace rb {
    /**
     * @brief Utility class to manage assets.
     */
    class assets {
    public:
        /**
         * @brief Add asset loader to the registry.
         * 
         * @tparam Asset Type of asset for loading.
         * @tparam Loader Type of loader. Must override calling operator.
         * @tparam Args Types of arguments to construct a new loader.
         * 
         * @param args Arguments to construct a new loader.
         */
        template<typename Asset, typename Loader, typename... Args>
        void loader(Args&&... args) {
            m_loaders.emplace(type_hash<Asset>(), [loader = std::make_shared<Loader>(std::forward<Args>(args)...)](std::string_view path) {
                return ref<Asset>((*loader)(path));
            });
        }

        /**
         * @brief Load a new asset. If asset is already loaded, returns that asset without loading.
         * 
         * @tparam Asset Type of asset to load.
         * 
         * @param path Path to the asset.
         * 
         * @return Loaded reference to asset.
         */
        template<typename Asset>
        ref<Asset> load(std::string_view path) {
            auto& loaded_asset = m_assets[std::string(path)];
            if (loaded_asset) {
                return ref<Asset>((Asset*)loaded_asset.get());
            }

            auto& loader = m_loaders.at(type_hash<Asset>());
            ref<Asset> new_asset = (Asset*)loader(path).get();
            loaded_asset = new_asset;
            return new_asset;
        }

    private:
        /**
         * @brief Registered loaders.
         */
        std::map<id_type, std::function<ref<reference>(std::string_view)>> m_loaders;

        /**
         * @brief Loaded assets.
         */
        std::unordered_map<std::string, ref<reference>> m_assets;
    };
}
