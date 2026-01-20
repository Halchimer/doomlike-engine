#pragma once

#include <hcgmath.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_image.h>

typedef struct texture_s {
    SDL_Surface *surface;
}texture_t;

texture_t load_image(const char *path);

u32 get_pixel(texture_t *texture, vec2 pos);
u32 get_pixel_from_index(texture_t *texture, i32 index);

void destroy_texture(texture_t *texture);
