# RabBit

`RabBit` is a easy to use, multiplatform engine for game develop written in **C++17**.

## Goals

- Provide platform implementations as many as possible.
- Producing high-quality modern code.
- Making a game engine as simple as possible.

## Requirements

To be able to use `RabBit`, users must provide a full-featured compiler that supports at least `C++17` and `CMake` version 3.16 or later.

## Code examples

### Hello world example

```cpp
#include <rabbit/rabbit.hpp>

using namespace rb;

int main(int argc, char* argv[]) {
    // Create new window.
    window window("hello_world", { 1280, 720 }, false);

    // Create renderer and attached window to it.
    renderer renderer(window);

    // Load texture from file using texture loader.
    texture texture = texture_loader(renderer)("data/characters.png");

    // Connect window close event to stop main loop.
    window.on<close_event>().connect<&window::close>(window);

    // Create stopwatch to use time factor.
    stopwatch stopwatch;

    // Run our example in loop until close button is pressed.
    while (window.is_open()) {
        // Dispatch window events and run all connected signals.
        window.dispatch_events();

        // Draw texture on screen.
        renderer.draw(texture, { 0, 32, 32, 32 }, { stopwatch.time() * 32.0f, 296.0f, 128.0f, 128.0f}, color::white());

        // Render and display it onto a screen.
        renderer.display(color::cornflower_blue());
    }
}
```

## License

Code released under [the MIT license](https://github.com/demurzasty/rabbit/blob/master/LICENSE).
