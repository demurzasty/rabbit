#pragma once

#include "fwd.hpp"
#include "../core/injector.hpp"

namespace rb {
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
    };
}
