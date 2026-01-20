#pragma once

#include <hclib.h>
#include <pthread.h>

#include "camera.h"
#include "level.h"
#include "renderer.h"
#include "../utils/utils.h"

// App State

#define UPDATE_ALLOCATOR_SIZE 4096
#define TICK_ALLOCATOR_SIZE 4096

typedef struct state_s {

    bool running;

    h_clock_t deltaclock;

// world

    level_t level;
    camera_t camera;

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

    pthread_t render_thread;
    pthread_t update_thread;

    state_t state;
    renderer_t renderer;

    struct layer_s *layer;
} app_t ;

app_t init_app(char const *name, i32 argc, char **argv);
void set_layer(app_t *app, const struct layer_s *layer);
i32 run_app(app_t *app);
void destroy_app(app_t *app);

void start(app_t *app);
void update(app_t *app);
