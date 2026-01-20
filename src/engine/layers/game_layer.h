#pragma once

#include "hcgmath.h"
#include "../core/layer.h"
#include "../core/raycast_renderer.h"

typedef struct game_state_s {
    vec2 movement;
} game_state_t;
extern game_state_t g_game_state;

void game_layer_start(void *state);
void game_layer_update(void *state, f64 dt);
void game_layer_tick(void *state, f64 dt);
void game_layer_destroy(void *state);

void game_event_handler(void *state, SDL_Event *event);

/**
 * @brief Defines the primary game layer for the application.
 *
 * The `game_layer` encapsulates the state and functionality for the gameplay layer.
 * It includes initialization, update, tick processing, rendering, destruction,
 * and event handling functionalities. The layer is associated with the global
 * game state (`g_game_state`) and uses specific function pointers to manage
 * lifecycle events and rendering.
 *
 * Components:
 * - `state`: Pointer to the global game state (`g_game_state`).
 * - `start`: Function called to initialize the game layer.
 * - `update`: Function called to handle per-frame updates with a delta time provided.
 * - `tick`: Function called for fixed interval updates with a delta time provided.
 * - `render`: Function called to render the game layer using a raycaster renderer.
 * - `destroy`: Function called to clean up resources and destroy the game layer.
 * - `event_handler`: Function called to handle SDL events specific to this layer.
 */
static const layer_t game_layer = (layer_t){
    .state = &g_game_state,
    .start = game_layer_start,
    .update = game_layer_update,
    .tick = game_layer_tick,
    .render = raycaster_render,
    .destroy = game_layer_destroy,
    .event_handler = game_event_handler
};