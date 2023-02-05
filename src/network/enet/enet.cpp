#include "enet.hpp"

#include <mutex>

using namespace rb;

static int count = 0;
static std::mutex mutex;

void rb::enet::retain() {
    std::unique_lock lock(mutex);

    if (count++ == 0) {
        [[maybe_unused]] int result = enet_initialize();
        assert(result == 0);
    }
}

void rb::enet::release() {
    std::unique_lock lock(mutex);

    if (--count == 0) {
        enet_deinitialize();
    }
}
