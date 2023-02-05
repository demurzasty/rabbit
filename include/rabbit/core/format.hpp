#pragma once 

#include <fmt/format.h>

namespace rb {
    using fmt::format;
    using fmt::print;
    using fmt::vprint;

    template<typename... Args>
    void println(Args&&... args) {
        print("{}\n", format(std::forward<Args>(args)...));
    }
}
