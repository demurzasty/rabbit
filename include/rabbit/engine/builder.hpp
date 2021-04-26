#pragma once

#include "fwd.hpp"
#include "../core/injector.hpp"

#include <vector>
#include <functional>

namespace rb {
    class builder {
        friend class application;

    public:
        template<typename T>
        builder& singleton() {
            _installations.push_back([](injector& injector) {
                injector.install<T>();
            });
            return *this;
        }

        template<typename Interface, typename Implementation>
        builder& singleton() {
            _installations.push_back([](injector& injector) {
                injector.install<Interface, Implementation>();
            });
            return *this;
        }

        template<typename T, typename Factory>
        builder& singleton(Factory factory) {
            _installations.push_back([factory](injector& injector) {
                injector.install<T>(factory);
            });
            return *this;
        }

        application build() const;

    private:
        std::vector<std::function<void(injector&)>> _installations;
    };
}
