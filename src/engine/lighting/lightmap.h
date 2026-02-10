#pragma once

#include <stddef.h>
#include <hclib.h>
#include <hcgmath.h>

// The distance interval between lightmap samples on a surface
constexpr f32 LIGHTMAP_SAMPLE_SPAN = 0.05f;

// Forward declarations
struct level_s;
struct segment_s;
struct sector_s;

// Data type of a single lightmap texel.
typedef u32 light_texel_t;

// Stores lighting data for a single surface (segment, floor or ceiling)
typedef struct lightmap_s {
    size_t w, h;
    light_texel_t *data;
} lightmap_t;

lightmap_t generate_segment_lightmap(h_arena_t *allocator, struct segment_s *seg, struct sector_s *sector);
lightmap_t generate_floor_lightmap(h_arena_t *allocator, struct sector_s *sector);
lightmap_t generate_ceiling_lightmap(h_arena_t *allocator, struct sector_s *sector);

typedef struct level_lightmap_s {
    h_arena_t *allocator;

    lightmap_t *light_data; // for each sector : floor, then ceil, then segments
    // for now only segments
    // TODO : add floor and ceiling lighting
}level_lightmap_t;

level_lightmap_t *generate_level_lightmap(h_arena_t *allocator, struct level_s *level);
void destroy_level_lightmap(level_lightmap_t *lightmap);