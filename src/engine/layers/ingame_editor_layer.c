#include "ingame_editor_layer.h"

#include "../core/app.h"
#include "../editor/nuklear_init.h"
#include "../core/level.h"
#include "../editor/level_editor_panels.h"

void ingame_editor_layer_start(void *state) {
    SDL_ShowCursor();
    SDL_SetWindowRelativeMouseMode(g_renderer->window.sdl_window, false);
}

void ingame_editor_layer_resume(void *state) {
}

void ingame_editor_layer_update(void *state, double dt) {}
void ingame_editor_layer_tick(void *state, double dt) {}
void ingame_editor_layer_render(void *state, struct renderer_s *renderer) {
    sector_t *sector = h_array_get(&g_state->level.sections, g_camera->sector->sector);

    draw_sector_edit_panel(sector);
}
void ingame_editor_layer_destroy(void *state) {}
void ingame_editor_event_handler(void *state, SDL_Event *event) {}