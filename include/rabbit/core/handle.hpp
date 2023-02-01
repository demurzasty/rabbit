#pragma once 

#include <entt/entt.hpp>

namespace rb {
    using entt::id_type;
    using entt::null;
    using entt::null_t;

    /**
     * @brief Multi-purpose object handle.
     *        Mainly used for communication with data-oriented services.
     *        (e.g. Renderer or Physics).
     */
    enum class handle : id_type {};
}
