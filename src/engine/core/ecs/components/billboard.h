#pragma once

#include <stddef.h>

typedef struct billboard_s {
    size_t section;
    float z; // height of the billboard relative to the floor of the current sector
} billboard_t;