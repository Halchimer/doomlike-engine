#include "../layers/game_layer.h"

#include "../core/app.h"
#include "../core/camera.h"
#include "../utils/utils.h"

game_state_t g_game_state = {0};

void game_layer_start(void *state) {
    SDL_HideCursor();
    SDL_SetWindowRelativeMouseMode(g_renderer->window.sdl_window, true);
}
void game_layer_update(void *state, f64 dt) {
    const vec2 forward = camera_forward(g_camera);
    const vec2 right = (vec2){-forward[1], forward[0]};

    game_state_t *gamestate = (game_state_t *)state;

    const bool *keystates = SDL_GetKeyboardState(NULL);

    gamestate->movement = (vec2){0.0, 0.0};

    if (keystates[SDL_SCANCODE_W])
        gamestate->movement = gamestate->movement + forward;
    if (keystates[SDL_SCANCODE_S])
        gamestate->movement = gamestate->movement - forward;
    if (keystates[SDL_SCANCODE_A])
        gamestate->movement = gamestate->movement + right;
    if (keystates[SDL_SCANCODE_D])
        gamestate->movement = gamestate->movement - right;
    if (gamestate->movement[0] != 0 || gamestate->movement[1] != 0)
        gamestate->movement = normalize(gamestate->movement);

    float mouseDelta[2] = {0.0f, 0.0f};
    SDL_GetRelativeMouseState(&mouseDelta[0], &mouseDelta[1]);
        g_camera->rotation -= mouseDelta[0] * dt * 5.0f;;

    vec2 vel = gamestate->movement * (f32)dt * 15.0f;

    // Collisions
    // TODO: Fix collisions with true velocity collision detection instead of input movement velocity + add player width

    section_t *section = h_array_get(&g_state->level.sections, g_camera->sector);
    h_iter_t segiter = h_array_iter(&g_state->level.segments);
    vec2 norm = {NAN, NAN};
    i32 portal = 0;
    for (segment_t *seg = section->first_segment; seg < section->first_segment + section->num_segments; ++seg) {
        vec2 inter = intersect_seg(g_camera->position, g_camera->position + vel, *seg->vertices[0], *seg->vertices[1]);
        if (isnan(inter[0])) continue;
        norm = get_segment_normal(seg);
        portal = seg->portal;
        break;
    }

    if (!isnan(norm[0]) && !portal) {
        vel = vel - norm * (f32)dot(vel, norm);
    }

    g_camera->position = g_camera->position + vel;

    if (portal)
        g_camera->sector = portal - 1;
}
void game_layer_tick(void *state, f64 dt) {

}
void game_layer_destroy(void *state) {
}

void game_event_handler(void *state, SDL_Event *event) {
    game_state_t *gamestate = (game_state_t *)state;


}
