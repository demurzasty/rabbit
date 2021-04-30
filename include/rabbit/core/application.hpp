
#pragma once

#include "injector.hpp"
#include "system.hpp"
#include "entity.hpp"

#include <vector>

namespace rb {
    class builder;

    class application {
    public:
        application(const builder& builder);

        application(const application&) = delete;
        application(application&&) = default;

        application& operator=(const application&) = delete;
        application& operator=(application&&) = default;

        int run();

    private:
        injector _injector;
        registry _registry;
        std::vector<std::shared_ptr<rb::system>> _systems;
    };
}
