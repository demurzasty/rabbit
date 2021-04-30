
#include <rabbit/loaders/mesh_loader.hpp>
#include <rabbit/core/config.hpp>
#include <rabbit/graphics/mesh.hpp>
#include <rabbit/math/vec2.hpp>
#include <rabbit/math/vec3.hpp>

#include <fstream>
#include <sstream>
#include <vector>

// TODO: Add indices writing (generate if necessary)
// TODO: Generate indices and optimize mesh.
// TODO: Add more formats (.dae, .gltf)

using namespace rb;

mesh_loader::mesh_loader(graphics_device& graphics_device)
    : _graphics_device(graphics_device) {
}

std::shared_ptr<void> mesh_loader::load(const std::string& filename, const json& metadata) {
    std::ifstream istream{ filename, std::ios::in };
    RB_ASSERT(istream.is_open(), "Cannot open file");

    struct mesh_vertex {
        vec3f position;
        vec2f texcoord;
        vec3f normal;
    };

    std::vector<mesh_vertex> vertices;
    std::vector<vec3f> positions;
    std::vector<vec2f> texcoords;
    std::vector<vec3f> normals;

    std::string line;
    while (std::getline(istream, line)) {
        std::istringstream iss{ line };
        std::vector<std::string> results{ std::istream_iterator<std::string>{ iss }, std::istream_iterator<std::string>{} };

        if (!results.empty()) {
            if (results[0] == "v") {
                positions.push_back({
                    static_cast<float>(std::atof(results[1].c_str())),
                    static_cast<float>(std::atof(results[2].c_str())),
                    static_cast<float>(std::atof(results[3].c_str()))
                });
            } else if (results[0] == "vn") {
                normals.push_back({
                    static_cast<float>(std::atof(results[1].c_str())),
                    static_cast<float>(std::atof(results[2].c_str())),
                    static_cast<float>(std::atof(results[3].c_str()))
                });
            } else if (results[0] == "vt") {
                texcoords.push_back({
                    static_cast<float>(std::atof(results[1].c_str())),
                    1.0f - static_cast<float>(std::atof(results[2].c_str())),
                });
            } else if (results[0] == "f") {
                int a, b, c;
                for (int i = 1; i < 4; ++i) {
                    sscanf(results[i].c_str(), "%d/%d/%d", &a, &b, &c);

                    vertices.push_back({
                        positions[a - 1],
                        texcoords[b - 1],
                        normals[c - 1]
                    });
                }
            }
        }
    }

    vertex_layout vertex_layout{
        { vertex_attribute::position, vertex_format::vec3f() },
        { vertex_attribute::texcoord, vertex_format::vec2f() },
        { vertex_attribute::normal, vertex_format::vec3f() }
    };

    mesh_desc desc;
    desc.topology = topology::triangles;
    desc.vertex_layout = vertex_layout;
    desc.vertex_data = vertices.data();
    desc.vertex_count = vertices.size();

    return _graphics_device.create_mesh(desc);
}
