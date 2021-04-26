#pragma once

#include "config.hpp"

#include <map>
#include <memory>
#include <typeinfo>
#include <typeindex>
#include <type_traits>
#include <functional>

namespace rb {
    /**
     * @brief Container of dependencies.
     */
    class injector {
        struct bean {
            std::shared_ptr<void> instance;
            std::function<std::shared_ptr<void>(injector&)> factory;
        };

        template<typename T>
        struct resolver {
            template<typename U, std::enable_if_t<!std::is_same_v<std::decay_t<T>, std::decay_t<U>>, int> = 0>
            operator U&() {
                return injector.get<U>();
            }

            injector& injector;
        };

    public:
        /**
         * @brief Install dependency.
         */
        template<typename T>
        void install() {
            _beans.emplace(typeid(T), bean{
                nullptr,
                [](injector& injector) { return injector.resolve<T>(); }
            });
        }

        /**
         * @brief Install dependency..
         */
        template<typename Interface, typename Implementation>
        void install() {
            _beans.emplace(typeid(Interface), bean{
                nullptr,
                [](injector& injector) { return injector.resolve<Implementation>(); }
            });
        }

        /**
         * @brief Install dependency using factory.
         */
        template<typename T, typename Func>
        void install(Func factory) {
            _beans.emplace(typeid(T), bean{
                nullptr,
                [factory](injector& injector) { return factory(injector); }
            });
        }

        /**
         * @brief Install dependency with provided instance.
         */
        template<typename T>
        void install(const std::shared_ptr<T>& instance) {
            _beans.emplace(typeid(T), bean{
                instance,
                nullptr
            });
        }

        /**
         * @brief Creates instance of a class with automatic resolved arguments.
         */
        template<typename T>
        std::shared_ptr<T> resolve() {
            using resolver = resolver<T>;

            resolver r{ *this };
            if constexpr (std::is_constructible_v<T>) {
                return std::make_shared<T>();
            } else if constexpr (std::is_constructible_v<T, resolver>) {
                return std::make_shared<T>(r);
            } else if constexpr (std::is_constructible_v<T, resolver, resolver>) {
                return std::make_shared<T>(r, r);
            } else if constexpr (std::is_constructible_v<T, resolver, resolver, resolver>) {
                return std::make_shared<T>(r, r, r);
            } else if constexpr (std::is_constructible_v<T, resolver, resolver, resolver, resolver>) {
                return std::make_shared<T>(r, r, r, r);
            }

            RB_ASSERT(false, "Cannot find suitable constructor");
            return nullptr;
        }

        /**
         * @brief Invoke invocable object with automatic resolved arguments.
         */
        template<typename Func>
        void invoke(Func func) {
            using resolver = resolver<T>;

            resolver r{ *this };
            if constexpr (std::is_invocable_v<Func>) {
                func();
            } else if constexpr (std::is_invocable_v<Func, resolver>) {
                func(r);
            } else if constexpr (std::is_invocable_v<Func, resolver, resolver>) {
                func(r, r);
            } else if constexpr (std::is_invocable_v<Func, resolver, resolver, resolver>) {
                func(r, r, r);
            } else if constexpr (std::is_invocable_v<Func, resolver, resolver, resolver, resolver>) {
                func(r, r, r, r);
            } else {
                RB_ASSERT(false, "Cannot find suitable invocation");
            }
        }

        /**
         * @brief Returns dependency.
         */
        template<typename T>
        T& get() {
            RB_ASSERT(installed<T>(), "Type is not installed");
            auto& bean = _beans.at(typeid(T));
            if (!bean.instance) {
                bean.instance = bean.factory(*this);
            }
            return *std::static_pointer_cast<T>(bean.instance);
        }

        template<>
        injector& get() {
            return *this;
        }

        /**
         * @brief Tell whether type is installed.
         */
        template<typename T>
        bool installed() const {
            return _beans.find(typeid(T)) != _beans.end();
        }

        /**
         * @brief Tell whether type is already resolved.
         */
        template<typename T>
        bool resolved() const {
            RB_ASSERT(installed<T>(), "Type is not installed");
            return _beans.at(typeid(T)).instance != nullptr;
        }

    private:
        std::map<std::type_index, bean> _beans;
    };
}
