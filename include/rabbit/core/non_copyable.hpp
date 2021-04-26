#pragma once

#include "config.hpp"

namespace rb {
    struct RB_NOVTABLE non_copyable {
        non_copyable() = default;

        non_copyable(const non_copyable&) = delete;
        non_copyable(non_copyable&&) = delete;

        non_copyable& operator=(const non_copyable&) = delete;
        non_copyable& operator=(non_copyable&&) = delete;
    };
}
