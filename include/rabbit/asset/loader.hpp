
#pragma once

#include "../core/json.hpp"

#include <string>
#include <memory>

namespace rb {
    class loader {
    public:
        /**
         * @brief Default virtual destructor.
         */
        virtual ~loader() = default;

        /**
         * @brief Loads new resource.
         *
         * @param filename Path to resource.
         * @param metadata Additional metadata info if corresponding .meta file exists, otherwise is empty.
         *
         * @returns Loaded resource.
         */
        virtual std::shared_ptr<void> load(const std::string& filename, const json& metadata) = 0;
    };
}
