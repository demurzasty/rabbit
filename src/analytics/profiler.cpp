#include <rabbit/analitycs/profiler.hpp>

using namespace rb;

void profiler::record(entt::hashed_string::hash_type id,
    const char* name,
    const std::chrono::high_resolution_clock::time_point::duration& duration) {
    auto& record = _records[id];
    record.name = name;
    record.duration = duration;
}

const std::unordered_map<entt::hashed_string::hash_type, profiler_record>& profiler::records() const {
    return _records;
}
