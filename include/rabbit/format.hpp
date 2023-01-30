#pragma once 

#include <fmt/format.h>

namespace rb {
    using fmt::format;
    using fmt::print;

    template<typename T, typename... Args>
    void println(T&& p_format, Args&&... p_args) {
        print("{}\n", format(std::forward<T>(p_format), std::forward<Args>(p_args)...));
    }
}
