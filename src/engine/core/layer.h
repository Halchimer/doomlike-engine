#pragma once

#include <hclib.h>

#include "SDL3/SDL_events.h"

struct renderer_s;

/*
 *  Layered app architecture, for game, 3d editor & 2d editor
 */

typedef struct layer_s {
    void *state;
    void (*start)(void *state);
    void (*update)(void *state, f64 dt);
    void (*tick)(void *state, f64 dt);
    void (*render)(void *state, struct renderer_s *renderer);
    void (*destroy)(void *state);

    void (*event_handler)(void *state, SDL_Event *event);
} layer_t;

/*

    To create an app layer do as such :
    const layer_t MyLayer = (layer_t){
        .state = &layer_state,
        .start = layer_start,
        ...
    };

 */