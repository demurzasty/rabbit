#pragma once 

#include "../core/json.hpp"

#include <string_view>

namespace rb {
    /**
     * @brief Basically, you can just use json directly,
     *        but for convenient we create another utility class
     *        to fit others loaders.
     *          
     *        Also, we hide huge <fstream> headers in source file.
     */
    class json_loader {
    public:
        /**
         * @brief Load new json from file.
         *
         * @return Loaded json.
         */
        [[nodiscard]] json operator()(std::string_view path) const;
    };
}
