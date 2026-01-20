#include "texture.h"

#include <stdlib.h>

texture_t load_image(const char *path) {
    SDL_Surface *surface = IMG_Load(path);
    return (texture_t){surface};
}

u32 get_pixel(texture_t *texture, vec2 pos) {
    if (pos[0] >= texture->surface->w || pos[1] >= texture->surface->h) return 0xFF00FFFF;
    u8 r, g, b, a;
    SDL_ReadSurfacePixel(texture->surface, pos[0], pos[1], &r, &g, &b, &a);
    return (r << 24) | (g << 16) | (b << 8) | (a << 0);
}

u32 get_pixel_from_index(texture_t *texture, i32 index) {
    if (index > texture->surface->w * texture->surface->h) return 0xFF00FFFF;
    return ((u32*)texture->surface->pixels)[index];
}

void destroy_texture(texture_t *texture) {
    free(texture->surface);
    texture->surface = NULL;
}
