#pragma once

#include "../core/non_copyable.hpp"

#include <entt/config/config.h>
#include <entt/core/hashed_string.hpp>

#include <chrono>
#include <unordered_map>
#include <string_view>

#define RB_PROFILE(profiler, name) rb::profiler_guard<entt::hashed_string::value(name)> __profiler_guard__{ profiler, name }

namespace rb {
    struct profiler_record {
        const char* name;
        std::chrono::high_resolution_clock::time_point::duration duration;
    };

    class profiler : public non_copyable {
    public:
        void record(entt::hashed_string::hash_type id,
            const char* name,
            const std::chrono::high_resolution_clock::time_point::duration& duration);

        const std::unordered_map<entt::hashed_string::hash_type, profiler_record>& records() const;

    private:
        std::unordered_map<entt::hashed_string::hash_type, profiler_record> _records;
    };

    template<entt::hashed_string::hash_type id>
    class profiler_guard {
    public:
        profiler_guard(profiler& profiler, const char* name)
            : _profiler(profiler)
            , _name(name)
            , _start_time(std::chrono::high_resolution_clock::now()) {
        }

        ~profiler_guard() {
            const auto duration = std::chrono::high_resolution_clock::now() - _start_time;
            _profiler.record(id, _name, duration);
        }

    private:
        profiler& _profiler;
        const char* _name;
        std::chrono::high_resolution_clock::time_point _start_time;
    };
}