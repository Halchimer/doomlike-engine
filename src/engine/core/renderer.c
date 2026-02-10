#include "../core/renderer.h"
#include "app.h"
#include "ecs/components/camera.h"
#include "../utils/version.h"

renderer_t *g_renderer;

#define SCREEN_RES_FACTOR 0.25

renderer_result_t init_renderer(struct app_s *app,i32 w,i32 h) {
    SDL_SetAppMetadata(app->name, DOOMLIKE_VERSION, app->name);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return (renderer_result_t){.isA=false,.b=SDL_APP_FAILURE};
    }

    renderer_t renderer = {0};

    renderer.w = w * SCREEN_RES_FACTOR;
    renderer.h = h * SCREEN_RES_FACTOR;

    window_result_t winres = init_window(app->name,w, h);
    if (!winres.isA) return (renderer_result_t){.isA=false,.b=winres.b};
    renderer.window = winres.a;

    renderer.sdl_renderer = SDL_CreateRenderer(renderer.window.sdl_window, nullptr);
    if (!renderer.sdl_renderer) {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        return (renderer_result_t){.isA=false,.b=SDL_APP_FAILURE};
    }
    SDL_SetRenderLogicalPresentation(renderer.sdl_renderer, w, h, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    renderer.render_texture = SDL_CreateTexture(renderer.sdl_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, w*SCREEN_RES_FACTOR, h*SCREEN_RES_FACTOR);
    SDL_SetTextureScaleMode(renderer.render_texture, SDL_SCALEMODE_PIXELART);
    printf("Creating pixel buffer...");
    renderer.pixels = h_arena_alloc(app->state.global_arena, (w*SCREEN_RES_FACTOR)*(h*SCREEN_RES_FACTOR)*sizeof(u32));
    printf("Creating depth buffer...");
    renderer.depth_buffer = h_arena_alloc(app->state.global_arena, (w*SCREEN_RES_FACTOR)*(h*SCREEN_RES_FACTOR)*sizeof(_Float16));
    printf("Creating ray directions buffer...");
    renderer.ray_directions = h_arena_alloc(app->state.global_arena, (w*SCREEN_RES_FACTOR)*sizeof(vec2));

    return (renderer_result_t){.isA=true,.a=renderer};
}

void renderer_set_global(renderer_t *renderer) {
    g_renderer = renderer;
    g_window = &renderer->window;
}

void render_frame(renderer_t *renderer) {

}

void destroy_renderer(renderer_t *renderer) {
    SDL_DestroyTexture(renderer->render_texture);
    SDL_DestroyRenderer(renderer->sdl_renderer);
}

void precompute_ray_directions(renderer_t *renderer) {
    if (!g_camera) return;
    for (i32 x = 0; x < renderer->w; ++x) {
        float rx = (float)x / renderer->w;
        float dm = 2.0f * (rx - 0.5f);
        dm *= tanf(g_camera->fov) * g_camera->near;
        vec2 w = (vec2){dm, 1.0f};
        renderer->ray_directions[x] = normalize(w);
    }
}

void fill_pixels(renderer_t *renderer, u32 color) {
    memset(renderer->pixels, color, renderer->w * renderer->h * sizeof(u32));
}

void draw_vert_line(renderer_t *renderer, i32 x, i32 y0, i32 y1, u32 color) {
    for (i32 y = y0; y <= y1; ++y) {
        i32 idx = y * renderer->w + x;
        if (idx < 0 || idx >= renderer->w * renderer->h) continue;
        renderer->pixels[idx] = color;
    }
}

void draw_vert_line_depth_buffer(renderer_t *renderer, i32 x, i32 y0, i32 y1, _Float16 depth) {
    for (i32 y = y0; y <= y1; ++y) {
        i32 idx = y * renderer->w + x;
        if (idx < 0 || idx >= renderer->w * renderer->h) continue;
        renderer->depth_buffer[idx] = depth;
    }
}

void draw_line(renderer_t *renderer, vec2 a, vec2 b, u32 color) {
    for (float t = 0; t <= 1; t += 0.01) {
        vec2 p = lerp(a, b, t);
        renderer->pixels[(int)p[1] * renderer->w + (int)p[0]] = color;
    }
}
