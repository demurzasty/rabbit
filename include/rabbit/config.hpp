#pragma once 

#include <entt/entt.hpp>

#include <limits>
#include <type_traits>

namespace rb {
    using entt::id_type;
    using entt::null;

    struct non_copyable {
        non_copyable() = default;

        non_copyable(const non_copyable&) = delete;
        non_copyable& operator=(const non_copyable&) = delete;
    };
}
