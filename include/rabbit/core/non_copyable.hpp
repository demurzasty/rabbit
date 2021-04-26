#pragma once

namespace rb {
    struct non_copyable {
        non_copyable() = default;

        non_copyable(const non_copyable&) = delete;
        non_copyable(non_copyable&&) = delete;

        non_copyable& operator=(const non_copyable&) = delete;
        non_copyable& operator=(non_copyable&&) = delete;
    };
}
