#pragma once 

#include "config.hpp"
#include "memory.hpp"

#include <unordered_map>

namespace rb {
    class assets {
    public:

    private:
        std::unordered_map<std::string, std::shared_ptr<void>> m_assets;
    };
}