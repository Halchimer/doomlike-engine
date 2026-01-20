#pragma once

#include <SDL3/SDL.h>

#include "error.h"
#include "hcgmath.h"
#include "window.h"

struct app_s;

typedef struct renderer_s {
    i32 w,h;
    SDL_Renderer *sdl_renderer;

    SDL_Texture *render_texture;
    u32 *pixels;
    _Float16 *depth_buffer;

    window_t window;
} renderer_t;
extern renderer_t *g_renderer;

typedef RESULT(renderer_t) renderer_result_t;

renderer_result_t init_renderer(struct app_s *app,i32 w,i32 h);

void renderer_set_global(renderer_t *renderer);

void render_frame(renderer_t *renderer);

void destroy_renderer(renderer_t *renderer);

void fill_pixels(renderer_t *renderer,u32 color);

void draw_vert_line(renderer_t *renderer, i32 x, i32 y0, i32 y1, u32 color);

void draw_vert_line_depth_buffer(renderer_t *renderer, i32 x, i32 y0, i32 y1, _Float16 depth);

void draw_line(renderer_t *renderer, vec2 a, vec2 b, u32 color);