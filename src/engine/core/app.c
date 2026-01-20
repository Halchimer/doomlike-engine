#include "../core/app.h"

#include "layer.h"
#include "camera.h"
#include "../layers/game_layer.h"
#include "../layers/level_editor_layer.h"
#include "texture_atlas.h"
#include "../utils/utils.h"

state_t init_state() {
    state_t state = {0};
    state.global_arena = h_arena_create("global_arena");
    state.update_allocator = h_linear_allocator_create(UPDATE_ALLOCATOR_SIZE, "update_allocator");
    state.tick_allocator = h_linear_allocator_create(TICK_ALLOCATOR_SIZE, "tick_allocator");
    return state;
}

void destroy_state(state_t *state) {
    // world

    destroy_level(&state->level);

    // memory
    h_arena_destroy(state->global_arena);
    h_linear_allocator_destroy(state->update_allocator);
    h_linear_allocator_destroy(state->tick_allocator);
}

state_t *g_state;

app_t init_app(char const *name, i32 argc, char **argv) {
    app_t app = {name, argc, argv};
    app.state = init_state();
    app.renderer = init_renderer(&app, 800, 600).a;
    return app;
}

void set_layer(app_t *app, const struct layer_s *layer) {
    if (app->layer) app->layer->destroy(app->layer->state);
    app->layer = layer;
    if (app->state.running)
        layer->start(layer->state);
}

i32 run_app(app_t *app) {
    g_state = &app->state;

    start(app);
    app->layer->start(app->layer->state);

    app->state.running = true;

    clock_now(&app->state.deltaclock);
    while (app->state.running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                app->state.running = false;

            if (event.type == SDL_EVENT_KEY_UP) {
                if (event.key.key == SDLK_SPACE) {
                    if (app->layer == &level_editor_layer)
                        set_layer(app, &game_layer);
                    else
                        set_layer(app, &level_editor_layer);
                }
                if (event.key.key == SDLK_R) {
                    destroy_level(&g_state->level);
                    g_state->level = load_level("level.txt");
                }
            }

            app->layer->event_handler(app->layer->state, &event);
        }

        update(app);
        app->layer->update(app->layer->state, clock_delta(&app->state.deltaclock) / 1000.0);
        app->layer->render(app->layer->state, &app->renderer);

        h_linear_allocator_reset(app->state.update_allocator);
        clock_now(&app->state.deltaclock);

        char dstr[128];
        sprintf(dstr, "Doomlike | Delta : %.2fms | FPS : %.2f", clock_delta(&app->state.deltaclock), 1000 / clock_delta(&app->state.deltaclock));
        SDL_SetWindowTitle(app->renderer.window.sdl_window, dstr);
    }
    return 0;
}

void destroy_app(app_t *app) {
    app->layer->destroy(app->layer->state);
    destroy_state(&app->state);

    //for (int i = 0;i<MAX_TEXTURES;++i) destroy_texture(&g_texture_atlas[i]);
}

void start(app_t *app) {
    init_texture_atlas();
    g_state->level = load_level("level.txt");
    g_state->camera = init_camera((vec2){17, 8}, 0, 85.0 * TO_RADIANS, 0.1);
    g_camera = &g_state->camera;
    g_renderer = &app->renderer;
}

void update(app_t *app) {
}
