#pragma once

#include <hclib.h>
#include <hcgmath.h>

typedef struct segment_s {
    vec2 *vertices[2];
    i32 portal;
    i32 texid;
    f32 uvs[4];
} segment_t;

vec2 get_segment_normal(segment_t *segment);

typedef struct section_s {
    segment_t *first_segment;
    size_t num_segments;
    float floor, ceil;
    i32 fctexid[2];
    f32 fcuvs[4];
} section_t;

vec2 get_section_center(section_t *section);

typedef struct level_s {
    const char* level_name;
    h_array_t vertices;
    h_array_t segments;
    h_array_t sections;
} level_t;

level_t load_level(char const *path);

void destroy_level(level_t *level);