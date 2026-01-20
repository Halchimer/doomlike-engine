#include "texture_atlas.h"

texture_atlas_index_t get_atlas_index_from_name(const char *name) {
#define X(Name) if (strcmp(name, #Name) == 0) return Name + 1;
    ATLAS_TEXTURES(X)
#undef X
    return -1;
}

texture_t g_texture_atlas[MAX_TEXTURES];

void init_texture_atlas() {
    g_texture_atlas[WALL] = load_image("wall.png");
    g_texture_atlas[STONE] = load_image("StoneUWL.png");
    g_texture_atlas[SLAB] = load_image("026-slab1.png");
    g_texture_atlas[BRONZE] = load_image("005-doomhexbronze.png");
}