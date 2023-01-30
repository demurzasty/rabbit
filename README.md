# RabBit

`RabBit` is a easy to use, multiplatform engine for game develop written in **C++17**.

## Goals

- Provide platform implementations as many as possible.
- Producing high-quality modern code.
- Making a game engine as simple as possible.

## Requirements

To be able to use `RabBit`, users must provide a full-featured compiler that supports at least `C++17` and `CMake` version 3.16 or later.

## Code examples

### Simple window creation

```cpp
#include <rabbit/rabbit.hpp>

using namespace rb;

void on_close(bool& p_open) {
    p_open = false;
}

int main(int argc, char* argv[]) {
    window window;
    
    bool open = true;
    window.on_close().connect<&on_close>(open);

    while (open) {
        window.dispatch_events();
    }

    return 0;
}
```

## License

Code released under [the MIT license](https://github.com/demurzasty/rabbit/blob/master/LICENSE).
