#include <rabbit/engine/builder.hpp>
#include <rabbit/engine/application.hpp>

using namespace rb;

application builder::build() const {
    return *this;
}
