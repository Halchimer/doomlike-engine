#pragma once

#include <nuklear_sdl3_renderer.h>
#include <nuklear.h>

#include "../core/renderer.h"

typedef struct nuklear_instance_s {
    struct nk_context *ctx;
    struct nk_colorf bg;
} nuklear_instance_t;
extern nuklear_instance_t *g_nuklear_instance;

nuklear_instance_t init_nuklear_instance(renderer_t *renderer);
void destroy_nuklear_instance(nuklear_instance_t *instance);
void handle_nuklear_events(nuklear_instance_t *instance, SDL_Event *event);
void render_nuklear(nuklear_instance_t *instance);