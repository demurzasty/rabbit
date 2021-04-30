
#pragma once

#include "injector.hpp"
#include "system.hpp"
#include "application.hpp"
#include "settings.hpp"

#include "../asset/asset_manager.hpp"

#include <list>
#include <map>
#include <functional>

namespace rb {
    class builder {
        friend class application;

    public:
        static builder create_default(const settings& settings);

        builder(const settings& settings);

        builder(const builder&) = default;
        builder(builder&&) = default;

        builder& operator=(const builder&) = default;
        builder& operator=(builder&&) = default;

        template<typename Func>
        builder& configure(Func func) {
            _installations.emplace(0, [func](injector& injector) {
                injector.invoke(func);
            });
            return *this;
        }

        template<typename T>
        builder& singleton() {
            _installations.emplace(1, [](injector& injector) {
                injector.install<T>();
            });
            return *this;
        }

        template<typename T>
        builder& singleton(int priority) {
            _installations.emplace(priority, [](injector& injector) {
                injector.install<T>();
            });
            return *this;
        }

        template<typename Interface, typename Implementation>
        builder& singleton() {
            _installations.emplace(1, [](injector& injector) {
                injector.install<Interface, Implementation>();
            });
            return *this;
        }

        template<typename T, typename Factory>
        builder& singleton(Factory factory) {
            _installations.emplace(1, [factory](injector& injector) {
                injector.install<T>(factory);
            });
            return *this;
        }

        template<typename Func>
        builder& initialize(Func func) {
            _installations.emplace(2, [func](injector& injector) {
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
        std::multimap<int, std::function<void(injector&)>> _installations;
        std::list<std::function<std::shared_ptr<rb::system>(injector&)>> _system_factories;
    };
}