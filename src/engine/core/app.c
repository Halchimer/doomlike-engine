#include "../core/app.h"

#include "layer.h"
#include "ecs/components/camera.h"
#include "../layers/game_layer.h"
#include "../layers/level_editor_layer.h"
#include "texture_atlas.h"
#include "../layers/debug_info_layer.h"
#include "../utils/utils.h"

void create_player(app_t *app) {
    app->state.player = create_entity(&g_state->level.world);
    camera_t *cam = add_component(&g_state->level.world, app->state.player, CAMERA);
    transform_t *t = add_component(&g_state->level.world, app->state.player, TRANSFORM);
    t->position = (vec2){17, 8};
    t->rotation = 0;
    velocity_t *v = add_component(&g_state->level.world, app->state.player, VELOCITY);
    v->v = (vec2){0, 0};
    level_collider_t *l = add_component(&g_state->level.world, app->state.player, LEVEL_COLLIDER);
    l->radius = 0.2f;
    sector_component_t *s = add_component(&g_state->level.world, app->state.player, SECTOR_COMPONENT);
    s->sector = 0;
    *cam = init_camera(t, 85.0 * TO_RADIANS, 0.1);
    cam->sector = s;
    set_active_camera(cam);
}

state_t init_state() {
    state_t state = {0};
    state.global_arena = h_arena_create("global_arena");
    state.update_allocator = h_linear_allocator_create(UPDATE_ALLOCATOR_SIZE, "update_allocator");
    state.tick_allocator = h_linear_allocator_create(TICK_ALLOCATOR_SIZE, "tick_allocator");
    return state;
}

void destroy_state(state_t *state) {
    // memory
    h_arena_destroy(state->global_arena);
    h_linear_allocator_destroy(state->update_allocator);
    h_linear_allocator_destroy(state->tick_allocator);

    // level
    destroy_level(&state->level);
}

state_t *g_state;

app_t init_app(char const *name, i32 argc, char **argv) {
    app_t app = {name, argc, argv};
    app.state = init_state();
    app.renderer = init_renderer(&app, 800, 600).a;
    return app;
}

void set_layer(app_t *app, const struct layer_s *layer) {
    if (!app || !layer) return;

    for (int x=0;x<app->num_layers;++x)
        app->layer_stack[x]->destroy(app->layer_stack[x]->state);

    app->layer_stack[0] = layer;
    app->num_layers = 1;

    if (app->state.running)
        app->layer_stack[0]->start(app->layer_stack[0]->state);
}

void add_layer(app_t *app, const struct layer_s *layer) {
    if (!layer || !app) return;

    if (app->num_layers >= MAX_APP_LAYERS ) {
        fprintf(stderr, "Too many app layers, consider increasing MAX_APP_LAYERS or popping some layers.\n");
        return;
    }

    app->layer_stack[app->num_layers++] = layer;
    if (app->state.running)
        app->layer_stack[app->num_layers-1]->start(app->layer_stack[app->num_layers-1]->state);
}

void pop_layer(app_t *app) {
    if (!app) return;
    if (app->num_layers == 0) return;

    app->layer_stack[app->num_layers-1]->destroy(app->layer_stack[app->num_layers-1]->state);
    --app->num_layers;
}

i32 run_app(app_t *app) {
    g_state = &app->state;

    start(app);
    for (int x=0;x<app->num_layers;++x)
        app->layer_stack[x]->start(app->layer_stack[x]->state);

    app->state.running = true;

    clock_now(&app->state.deltaclock);
    while (app->state.running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                app->state.running = false;

            if (event.type == SDL_EVENT_KEY_UP) {
                if (event.key.key == SDLK_SPACE) {
                    if (app->layer_stack[0] == &level_editor_layer) {
                        set_layer(app, &game_layer);
                        add_layer(app, &debug_info_layer);
                    }
                    else
                        set_layer(app, &level_editor_layer);
                }
                if (event.key.key == SDLK_R) {
                    destroy_level(&g_state->level);
                    g_state->level = load_level("level.txt");
                    create_player(app);
                }
                if (event.key.key == SDLK_L) {
                    destroy_level_lightmap(g_state->level.lightmap);
                    g_state->level.lightmap = generate_level_lightmap(g_state->level.level_arena, &g_state->level);
                }

            }

            // If an app layer has 'consume events' to true, ignore inputs for all layers bellow.
            for (int x=app->num_layers-1;x>=0;--x) {
                app->layer_stack[x]->event_handler(app->layer_stack[x]->state, &event);
                if (app->layer_stack[x]->consume_events)
                    break;
            }
        }

        update(app);
        for (int x=0;x<app->num_layers;++x) {
            app->layer_stack[x]->update(app->layer_stack[x]->state, clock_delta(&app->state.deltaclock) / 1000.0);
            app->layer_stack[x]->render(app->layer_stack[x]->state, &app->renderer);
        }

        SDL_RenderPresent(app->renderer.sdl_renderer);

        h_linear_allocator_reset(app->state.update_allocator);
        clock_now(&app->state.deltaclock);

        char dstr[128];
        sprintf(dstr, "Doomlike | Delta : %.2fms | FPS : %.2f", clock_delta(&app->state.deltaclock), 1000 / clock_delta(&app->state.deltaclock));
        SDL_SetWindowTitle(app->renderer.window.sdl_window, dstr);
    }
    return 0;
}

void destroy_app(app_t *app) {
    for (int x=0;x<app->num_layers;++x)
        app->layer_stack[x]->destroy(app->layer_stack[x]->state);
    destroy_state(&app->state);

    //for (int i = 0;i<MAX_TEXTURES;++i) destroy_texture(&g_texture_atlas[i]);
}

void start(app_t *app) {
    init_texture_atlas();
    g_state->level = load_level("level.txt");
    create_player(app);
    precompute_ray_directions(&app->renderer);
    g_renderer = &app->renderer;
}

void update(app_t *app) {
}
