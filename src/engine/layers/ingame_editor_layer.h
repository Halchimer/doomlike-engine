#pragma once

#include "../core/layer.h"

void ingame_editor_layer_start(void *state);
void ingame_editor_layer_resume(void *state);
void ingame_editor_layer_update(void *state, double dt);
void ingame_editor_layer_tick(void *state, double dt);
void ingame_editor_layer_render(void *state, struct renderer_s *renderer);
void ingame_editor_layer_destroy(void *state);
void ingame_editor_event_handler(void *state, SDL_Event *event);

static const layer_t ingame_editor_layer = {
    .state = nullptr,
    .start = ingame_editor_layer_start,
    .resume = ingame_editor_layer_resume,
    .update = ingame_editor_layer_update,
    .tick = ingame_editor_layer_tick,
    .render = ingame_editor_layer_render,
    .destroy = ingame_editor_layer_destroy,
    .event_handler = ingame_editor_event_handler,
    .consume_events = true,
    .consume_update = true
};