#include <rabbit/loaders/json_loader.hpp>

#include <fstream>

using namespace rb;

json json_loader::operator()(std::string_view path) const {
    std::ifstream stream(std::string(path), std::ios::in);
    assert(stream.is_open());

    json json;
    stream >> json;
    return json;
}
