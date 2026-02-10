#pragma once

#include <hclib.h>
#include <hcgmath.h>

#include "ecs/ecs.h"

struct level_s;
struct lightmap_s;

typedef struct segment_s {
    i32 vertices[2];
    i32 portal;
    i32 texid;
    f32 uvs[4];
} segment_t;

void get_vertices(struct level_s *level, segment_t *segment, vec2* verts[2]);
vec2 get_segment_normal(struct level_s *level,segment_t *segment);

typedef struct sector_s {
    segment_t *first_segment;
    size_t num_segments;
    float floor, ceil;
    i32 fctexid[2];
    f32 fcuvs[4];

    vec4 bounds; // bounding box of a sector

    struct lightmap_s *lightmaps;
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

    // Graphics
    level_lightmap_t *lightmap;
} level_t;

level_t load_level(char const *path);

void destroy_level(level_t *level);