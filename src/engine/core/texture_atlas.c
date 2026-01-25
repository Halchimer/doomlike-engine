#include "texture_atlas.h"

texture_t g_texture_atlas[MAX_TEXTURES];

void init_texture_atlas() {
    g_texture_atlas[STONE] = load_image("assets/BRICK_1A.PNG");
    g_texture_atlas[SLAB] = load_image("assets/TILE_1A.PNG");
    g_texture_atlas[HEALER] = load_image("assets/healer.gif");
}