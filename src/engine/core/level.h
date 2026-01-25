#pragma once

#include <hclib.h>
#include <hcgmath.h>

#include "ecs/ecs.h"

typedef struct segment_s {
    i32 vertices[2];
    i32 portal;
    i32 texid;
    f32 uvs[4];
} segment_t;

struct level_s;
void get_vertices(struct level_s *level, segment_t *segment, vec2* verts[2]);
vec2 get_segment_normal(struct level_s *level,segment_t *segment);

typedef struct sector_s {
    segment_t *first_segment;
    size_t num_segments;
    float floor, ceil;
    i32 fctexid[2];
    f32 fcuvs[4];
} sector_t;

vec2 get_section_center(struct level_s *level,sector_t *section);

typedef struct level_s {
    const char* level_name;

    h_arena_t *level_arena;

    h_array_t vertices;
    h_array_t segments;
    h_array_t sections;
    // ECS
    world_t world;
} level_t;

level_t load_level(char const *path);

void destroy_level(level_t *level);