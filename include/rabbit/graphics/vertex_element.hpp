#pragma once

#include "vertex_attribute.hpp"
#include "vertex_format.hpp"

namespace rb {
    struct vertex_element {
        vertex_attribute attribute{ vertex_attribute::position };
        vertex_format format;
    };
}
