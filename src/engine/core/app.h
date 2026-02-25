#pragma once

#include <hclib.h>
#include <pthread.h>

#include "layer.h"
#include "level.h"
#include "renderer.h"
#include "../utils/utils.h"
#include "../editor/nuklear_init.h"

#define MAX_APP_LAYERS 16

// App State

#define UPDATE_ALLOCATOR_SIZE 4096
#define TICK_ALLOCATOR_SIZE 4096

typedef struct state_s {

    bool running;

    h_clock_t deltaclock;

// world

    level_t level;
    entity_t player;

// memory
    h_arena_t *global_arena;

    h_linear_allocator_t *update_allocator;
    h_linear_allocator_t *tick_allocator;
} state_t ;
extern state_t *g_state;

state_t init_state();
void destroy_state(state_t *state);

// App

struct layer_s;

typedef struct app_s {
    char const *name;
    i32 argc;
    char **argv;
    state_t state;

    nuklear_instance_t nk;
    renderer_t renderer;

    struct layer_s *layer_stack[MAX_APP_LAYERS];
    size_t num_layers;
} app_t ;

app_t init_app(char const *name, i32 argc, char **argv);

///
/// Resets the app layer stack to the provided base layer.
///
/// @param app
/// @param layer
///
void set_layer(app_t *app, const struct layer_s *layer);

///
/// Push a layer to the app layer stack.
///
/// @param app
/// @param layer
void add_layer(app_t *app, const struct layer_s *layer);

///
/// Pops a layer from the app layer stack.
///
/// @param app
void pop_layer(app_t *app);

i32 run_app(app_t *app);
void destroy_app(app_t *app);

void start(app_t *app);
void update(app_t *app);
