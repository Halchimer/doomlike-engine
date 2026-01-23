#include "../layers/game_layer.h"

#include "../core/app.h"
#include "../core/ecs/components/camera.h"
#include "../utils/utils.h"

game_state_t g_game_state = {0};

void game_layer_start(void *state) {
    SDL_HideCursor();
    SDL_SetWindowRelativeMouseMode(g_renderer->window.sdl_window, true);
}
void game_layer_update(void *state, f64 dt) {
    const vec2 camfwd = camera_forward(g_camera);
    const float acceleration = 60.0f;
    const float max_speed = 30.0f;
    const float friction_factor = 5.0f;

    const vec2 forward = camera_forward(g_camera);
    const vec2 right = (vec2){-forward[1], forward[0]};

    velocity_t *player_vel = get_component(&g_state->level.world, g_state->player, VELOCITY);

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
        g_camera->transform->rotation -= mouseDelta[0] * dt * 5.0f;;

    player_vel->v = player_vel->v + gamestate->movement * acceleration * (f32)dt;
    player_vel->v = player_vel->v * (1.0f - friction_factor * (f32)dt);

    if (magnitude(player_vel->v) > max_speed)
        player_vel->v = normalize(player_vel->v) * max_speed;

    entity_query_t sector_colliders_query = query_entities(&g_state->level.world, 3, (component_id_t[]){TRANSFORM, VELOCITY, LEVEL_COLLIDER, SECTOR_COMPONENT});
    h_iter_t sector_colliders_iter = query_entities_iter(&sector_colliders_query);
    H_FOREACH(entity_t, e, sector_colliders_iter) {
        transform_t *t = get_component(&g_state->level.world, e, TRANSFORM);
        velocity_t *vel_comp = get_component(&g_state->level.world, e, VELOCITY);
        sector_component_t *sector_comp = get_component(&g_state->level.world, e, SECTOR_COMPONENT);
        level_collider_t *lcomp = get_component(&g_state->level.world, e, LEVEL_COLLIDER);

        if (!sector_comp) continue;

        sector_t *sec = h_array_get(&g_state->level.sections, sector_comp->sector);
        if (!sec) continue;
        struct intersect_circle_seg_result_s radius_inter = {false, NAN, NAN, NAN, NAN};
        vec2 inter = {NAN, NAN};
        segment_t *interseg = NULL;
        for (segment_t *seg = sec->first_segment; seg < sec->first_segment + sec->num_segments; ++seg) {

            radius_inter = intersect_circle_seg(t->position, lcomp->radius, *seg->vertices[0], *seg->vertices[1]);

            if (radius_inter.intersect && !seg->portal) {
                vec2 center = (radius_inter.a + radius_inter.b) / 2;
                if (magnitude(center - t->position) < lcomp->radius)
                    t->position = t->position - normalize(center - t->position) * (lcomp->radius - (f32)magnitude(center - t->position));
            }

            inter = intersect_seg(t->position, t->position + vel_comp->v * (f32)dt, *seg->vertices[0], *seg->vertices[1]);
            if (isnan(inter[0])) continue;

            if (seg->portal) {
                sector_comp->sector = seg->portal - 1;
                continue;
            }
            vec2 norm = get_segment_normal(seg);
            vel_comp->v = vel_comp->v - norm * (f32)dot(vel_comp->v, norm);

            break;
        }
    }

    entity_query_t vel_query = query_entities(&g_state->level.world, 2, (component_id_t[]){VELOCITY, TRANSFORM});
    h_iter_t vel_iter = query_entities_iter(&vel_query);
    H_FOREACH(entity_t, e, vel_iter) {
        velocity_t *vel_comp = get_component(&g_state->level.world, e, VELOCITY);
        transform_t *t = get_component(&g_state->level.world, e, TRANSFORM);
        t->position = t->position + vel_comp->v * (f32)dt;
    }

}
void game_layer_tick(void *state, f64 dt) {

}
void game_layer_destroy(void *state) {
}

void game_event_handler(void *state, SDL_Event *event) {
}
