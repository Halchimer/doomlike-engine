#pragma once

#include <hcgmath.h>
#include "../core/layer.h"

// TODO: Implement a level editor using nuklear as a GUI lib

typedef struct editor_layer_state_s {
    vec2 movement;
    vec2 pos;

} editor_layer_state_t;
extern editor_layer_state_t g_level_editor_state;

void editor_layer_start(void *state);
void editor_layer_update(void *state, f64 dt);
void editor_layer_tick(void *state, f64 dt);
void editor_layer_destroy(void *state);

void editor_layer_render(void *state, struct renderer_s *renderer);

void editor_event_handler(void *state, SDL_Event *event);

/**
 * @brief Defines the layer configuration for the level editor.
 *
 * This variable represents the implementation of the level editor layer
 * in the application. It includes function pointers that define the behavior
 * or lifecycle of the editor layer in terms of initialization, updates,
 * rendering, and destruction.
 *
 * The layer's lifecycle comprises the following members:
 * - `state`: Points to the state data specific to the level editor.
 * - `start`: Function called to start or initialize the layer.
 * - `update`: Function invoked during the update phase of the layer,
 *   allowing for things like gameplay logic or scene progression.
 * - `tick`: Function called regularly to handle lower-level, fixed-timestep operations.
 * - `render`: Function responsible for rendering the visual components
 *   of the level editor using the provided renderer.
 * - `destroy`: Function to clean up resources associated with the layer.
 * - `event_handler`: Function to process and respond to input or other events.
 */
static const layer_t level_editor_layer = {
    .state = &g_level_editor_state,
    .start = editor_layer_start,
    .update = editor_layer_update,
    .tick = editor_layer_tick,
    .render = editor_layer_render,
    .destroy = editor_layer_destroy,
    .event_handler = editor_event_handler
};