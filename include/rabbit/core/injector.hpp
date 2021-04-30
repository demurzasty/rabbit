#pragma once

#include "config.hpp"
#include "type_info.hpp"

#include <map>
#include <memory>
#include <functional>
#include <type_traits>

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
            static_assert(!std::is_abstract_v<T>, "Installation type must be non-abstract");

            _beans.emplace(type_id<T>().hash(), bean{
                nullptr,
                [](injector& injector) { return injector.resolve<T>(); }
            });
        }

        /**
         * @brief Install dependency..
         */
        template<typename Interface, typename Implementation>
        void install() {
            static_assert(!std::is_abstract_v<Implementation>, "Installation type must be non-abstract");
            static_assert(std::is_base_of_v<Interface, Implementation>, "Interface type must be base of Implementation");

            _beans.emplace(typeid(Interface), bean{
                nullptr,
                [](injector& injector) { return injector.resolve<Implementation>(); }
            });
        }

        /**
         * @brief Install dependency using factory.
         */
        template<typename T, typename Factory, std::enable_if_t<!std::is_same_v<std::decay_t<T>, std::decay_t<Factory>>, int> = 0>
        void install(Factory factory) {
            _beans.emplace(type_id<T>().hash(), bean{
                nullptr,
                [factory](injector& injector) { return factory(injector); }
            });
        }

        /**
         * @brief Install dependency with provided instance.
         */
        template<typename T>
        void install(const std::shared_ptr<T>& instance) {
            _beans.emplace(type_id<T>().hash(), bean{
                instance,
                nullptr
            });
        }
        /**
         * @brief Install dependency with provided instance.
         */
        template<typename T>
        void install(const T& instance) {
            _beans.emplace(type_id<T>().hash(), bean{
                std::make_shared<T>(instance),
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
            using resolver = resolver<void>;

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
            auto& bean = _beans.at(type_id<T>().hash());
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
            return _beans.find(type_id<T>().hash()) != _beans.end();
        }

        /**
         * @brief Tell whether type is already resolved.
         */
        template<typename T>
        bool resolved() const {
            RB_ASSERT(installed<T>(), "Type is not installed");
            return _beans.at(type_id<T>().hash()).instance != nullptr;
        }

    private:
        std::map<id_type, bean> _beans;
    };
}
