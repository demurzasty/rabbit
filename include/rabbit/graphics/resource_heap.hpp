#pragma once

#include "fwd.hpp"

#include <map>
#include <memory>
#include <variant>

namespace rb {
    using resource_view = std::variant<std::shared_ptr<buffer>, std::shared_ptr<texture>>;

    struct resource_heap_desc {
        std::shared_ptr<shader> shader;
        std::map<std::size_t, resource_view> resources;
    };

    class resource_heap {
    public:
        resource_heap(const resource_heap_desc& desc);

        virtual ~resource_heap() = default;

        const std::shared_ptr<shader>& associated_shader() const;

    private:
        const std::shared_ptr<shader> _shader;
    };
}

