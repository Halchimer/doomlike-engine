#include "ingame_editor_layer.h"

#include "../core/app.h"
#include "../editor/nuklear_init.h"
#include "../core/level.h"

void ingame_editor_layer_start(void *state) {
    SDL_ShowCursor();
    SDL_SetWindowRelativeMouseMode(g_renderer->window.sdl_window, false);
}

void ingame_editor_layer_resume(void *state) {
}

void ingame_editor_layer_update(void *state, double dt) {}
void ingame_editor_layer_tick(void *state, double dt) {}
void ingame_editor_layer_render(void *state, struct renderer_s *renderer) {
    struct nk_context *ctx = g_nuklear_instance->ctx;
    struct nk_colorf bg = g_nuklear_instance->bg;

    nk_input_end(ctx);

    sector_t *sector = h_array_get(&g_state->level.sections, g_camera->sector->sector);

    /* GUI */
    if (nk_begin(ctx, "Sector Editor", nk_rect(50, 50, 230, 250),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
        NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
    {
        nk_layout_row_dynamic(ctx, 25, 1);
        char buf[32];
        sprintf(buf, "Sector : %d", (int)(sector - (sector_t*)g_state->level.sections.data));
        nk_label(ctx, buf, NK_TEXT_ALIGN_CENTERED);
        nk_property_float(ctx, "Floor Height", 0, &sector->floor, sector->ceil, 0.01, 1);
        nk_property_float(ctx, "Ceiling Height", sector->floor, &sector->ceil, 100, 0.01, 1);
    }
    nk_end(ctx);

    nk_input_begin(ctx);
}
void ingame_editor_layer_destroy(void *state) {}
void ingame_editor_event_handler(void *state, SDL_Event *event) {}