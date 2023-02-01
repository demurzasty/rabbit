#include <rabbit/loaders/image_loader.hpp>

using namespace rb;

image image_loader::operator()(std::string_view path) const {
    return image::from(path);
}
