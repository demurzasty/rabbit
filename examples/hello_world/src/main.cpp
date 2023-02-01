#include <rabbit/rabbit.hpp>

#include <stdio.h>

using namespace rb;

struct foo {
    void create(int id) { printf("%d\n", id); }
    void destroy(int id) { printf("%d\n", id); }
};

template<auto Create, auto Destroy>
class resource {
public:
    resource(foo& foo) : m_foo(foo) {
        (m_foo.*Create)(5);
    }

    ~resource() {
        (m_foo.*Destroy)(3);
    }

private:
    foo& m_foo;
};

class foo_resource : public resource<&foo::create, &foo::destroy> {
public:
    foo_resource(foo& foo) : resource(foo) {}
};

int main(int argc, char* argv[]) {
    window window("hello_world", { 1280, 720 }, false);

    window.on<close_event>().connect<&window::close>(window);

    // texture texture = load();
    // texture.id(); 
    // renderer renderer;
    // texture texture = {renderer, renderer.create_texture() };
    // texture.size(); 
    foo foo;
    foo_resource resource{ foo };

    entt::registry registry;

    registry.emplace<int>(null);
    registry.destroy(null);

    while (window.is_open()) {
        window.wait_dispatch_events();

        // renderer.draw_sprite(texture, ); //

        // renderer.display();
    }
}
