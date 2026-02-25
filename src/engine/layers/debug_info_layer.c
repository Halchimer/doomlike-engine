#include "debug_info_layer.h"

#include "../core/app.h"
#include "../core/renderer.h"
#include "SDL3/SDL_render.h"

void debug_info_layer_start(void *state) {
}

void debug_info_layer_resume(void *state) {
}

void debug_info_layer_update(void *state, double dt) {
}

void debug_info_layer_tick(void *state, double dt) {
}

void debug_info_layer_render(void *state, struct renderer_s *renderer) {
    char fps_debug[16];
    sprintf(fps_debug, "FPS : %.2f", 1000.0f / fmaxf(clock_delta(&g_state->deltaclock), 0.1));
    SDL_RenderDebugText(renderer->sdl_renderer, 5, 5,fps_debug);

    char level_path_debug[128];
    sprintf(level_path_debug, "LEVEL : %s", g_state->level.level_name);
    SDL_RenderDebugText(renderer->sdl_renderer, 5, 20, level_path_debug);

    char current_sector_debug[32];
    sprintf(current_sector_debug, "CURRENT SECTOR : %d", g_camera->sector->sector);
    SDL_RenderDebugText(renderer->sdl_renderer, 5, 35, current_sector_debug);
}

void debug_info_layer_event_handler(void *state, SDL_Event *event) {
}

void debug_info_layer_destroy(void *state) {
}
