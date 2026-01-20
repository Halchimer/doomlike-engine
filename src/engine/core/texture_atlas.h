#pragma once

#include "texture.h"

// TODO: Implement a generic asset manager

// Macro trick to declare new atlas indices in a single place.
#define ATLAS_TEXTURES(X) \
    X(WALL)     \
    X(STONE)    \
    X(SLAB)     \
    X(BRONZE)

typedef enum texture_atlas_index_e {
#define X(Name) Name,
    ATLAS_TEXTURES(X)
#undef X
    MAX_TEXTURES
} texture_atlas_index_t;
texture_atlas_index_t get_atlas_index_from_name(const char *name);

extern texture_t g_texture_atlas[MAX_TEXTURES];

void init_texture_atlas();