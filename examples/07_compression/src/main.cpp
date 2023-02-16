#include <rabbit/rabbit.hpp>

using namespace rb;

int main(int argc, char* argv[]) {
    // Create new window.
    window window("basic_draw", { 1280, 720 }, false);

    // Create renderer and attached window to it.
    renderer renderer(window);

    // Create painter to dynamically render 2D stuff.
    painter painter(renderer, { 320, 180 });

    image image = image::from("data/buddy.png", true);

    compressor compressor;
    std::vector<std::uint8_t> compressed_data = compressor.compress(image.pixels());
    std::vector<std::uint8_t> uncompressed_data = compressor.uncompress(image.pixels().size(), compressed_data);

    texture texture(renderer, image.size(), texture_filter::nearest, pixel_format::rgba8);

    texture.update(uncompressed_data.data());

    // Connect window close event to stop main loop.
    window.on<close_event>().connect<&window::close>(window);

    // Run our example in loop until close button is pressed.
    while (window.is_open()) {
        // Dispatch window events and run all connected signals.
        window.dispatch();

        // Draw texture on screen.
        painter.draw(texture, { 146.0f, 60.0f }, color::white());

        // Render and display it onto a screen.
        renderer.display(color::cornflower_blue());
    }
}
