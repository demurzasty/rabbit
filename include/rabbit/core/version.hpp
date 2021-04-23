#pragma once

#define RB_VERSION_MAJOR 0
#define RB_VERSION_MINOR 1
#define RB_VERSION_PATCH 0

namespace rb {
    struct version {
        unsigned char major;
        unsigned char minor;
        unsigned char patch;
    };
}
