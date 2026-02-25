#include "nuklear_init.h"

nuklear_instance_t *g_nuklear_instance = nullptr;

nuklear_instance_t init_nuklear_instance(renderer_t *renderer) {
    nuklear_instance_t out;
    out.ctx = nk_sdl_init(renderer->window.sdl_window, renderer->sdl_renderer, nk_sdl_allocator());
    nk_sdl_style_set_debug_font(out.ctx);

    out.bg.r = 0.10f;
    out.bg.g = 0.18f;
    out.bg.b = 0.24f;
    out.bg.a = 1.0f;

    return out;
}
void destroy_nuklear_instance(nuklear_instance_t *instance) {
    nk_sdl_shutdown(instance->ctx);
}

void handle_nuklear_events(nuklear_instance_t *instance, SDL_Event *event) {
    nk_sdl_handle_event(instance->ctx, event);
}

void render_nuklear(nuklear_instance_t *instance) {
    nk_sdl_render(instance->ctx, NK_ANTI_ALIASING_OFF);
    nk_sdl_update_TextInput(instance->ctx);
}
