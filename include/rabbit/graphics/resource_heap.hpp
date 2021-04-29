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

    /**
     * @brief Resource heap interface.
     *
     * An instance of this interface provides all descriptor sets for graphics and compute pipelines.
     *
     * @see graphics_device::create_resource_heap
     * @see command_buffer::set_resource_heap
     */
    class resource_heap {
    public:
        resource_heap(const resource_heap_desc& desc);

        /**
         * @brief Default virtual destructor.
         */
        virtual ~resource_heap() = default;

        /**
         * @brief Returns associated shader provided with a descriptor.
         */
        const std::shared_ptr<shader>& associated_shader() const;

    private:
        const std::shared_ptr<shader> _shader;
        const std::map<std::size_t, resource_view> _resources;
    };
}

