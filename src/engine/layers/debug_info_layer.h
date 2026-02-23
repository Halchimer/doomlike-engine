#pragma once

#include "../core/layer.h"

void debug_info_layer_start(void *state);
void debug_info_layer_update(void *state, double dt);
void debug_info_layer_tick(void *state, double dt);
void debug_info_layer_render(void *state, struct renderer_s *renderer);
void debug_info_layer_event_handler(void *state, SDL_Event *event);
void debug_info_layer_destroy(void *state);


static const layer_t debug_info_layer = {
    .state = nullptr,
    .start = debug_info_layer_start,
    .update = debug_info_layer_update,
    .tick = debug_info_layer_tick,
    .render = debug_info_layer_render,
    .destroy = debug_info_layer_destroy,
    .event_handler = debug_info_layer_event_handler,
    .consume_events = false
};
