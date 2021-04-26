#pragma once

#include "fwd.hpp"
#include "../core/injector.hpp"
#include "../asset/asset_manager.hpp"

#include <list>
#include <functional>

namespace rb {
    class builder {
        friend class application;

    public:
        template<typename T>
        builder& singleton() {
            _installations.push_front([](injector& injector) {
                injector.install<T>();
            });
            return *this;
        }

        template<typename Interface, typename Implementation>
        builder& singleton() {
            _installations.push_front([](injector& injector) {
                injector.install<Interface, Implementation>();
            });
            return *this;
        }

        template<typename T, typename Factory>
        builder& singleton(Factory factory) {
            _installations.push_front([factory](injector& injector) {
                injector.install<T>(factory);
            });
            return *this;
        }

        template<typename Func>
        builder& initialize(Func func) {
            _installations.push_back([func](injector& injector) {
                injector.invoke(func);
            });
            return *this;
        }

        template<typename Asset, typename Loader>
        builder& loader() {
            return initialize([](asset_manager& asset_manager) {
                asset_manager.add_loader<Asset, Loader>();
            });
        }

        template<typename System>
        builder& system() {
            _system_factories.push_back([](injector& injector) {
                return injector.resolve<System>();
            });
            return *this;
        }

        application build() const;

    private:
        std::list<std::function<void(injector&)>> _installations;
        std::list<std::function<std::shared_ptr<rb::system>(injector&)>> _system_factories;
    };
}
