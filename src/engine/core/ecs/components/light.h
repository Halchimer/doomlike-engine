#pragma once

#include "../../../lighting/lightmap.h"

typedef struct light_s {
    vec4 tint;
    f32 intensity, radius, height;
} light_t;