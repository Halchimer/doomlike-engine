#include "level_editor_layer.h"

#include "../core/app.h"
#include "../core/renderer.h"
#include "SDL3/SDL_render.h"

// TODO: Implement a level editor using nuklear as a GUI lib

editor_layer_state_t g_level_editor_state = {
    .movement = (vec2){0, 0},
    .pos = (vec2){0, 0}
};

vec2 editor_world_to_screen(editor_layer_state_t *state, vec2 world) {
    return (vec2){
        world[0] * 10.0f - state->pos[0],
        world[1] * 10.0f - state->pos[1]
    };
}

void editor_layer_start(void *state) {
    editor_layer_state_t *estate = (editor_layer_state_t *)state;

    SDL_ShowCursor();
    SDL_SetWindowRelativeMouseMode(g_renderer->window.sdl_window, false);
}
void editor_layer_update(void *state, f64 dt) {
    editor_layer_state_t *estate = (editor_layer_state_t *)state;

    estate->pos = estate->pos + estate->movement * (f32)dt * 100.0f;
}
void editor_layer_tick(void *state, f64 dt) {

}
void editor_layer_destroy(void *state) {

}

void editor_layer_render(void *state, struct renderer_s *renderer) {
    editor_layer_state_t *estate = (editor_layer_state_t *)state;

    SDL_SetRenderDrawColor(renderer->sdl_renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer->sdl_renderer);

    h_iter_t segiter = h_array_iter(&g_state->level.segments);
    H_FOREACH(segment_t, seg, segiter) {
        SDL_SetRenderDrawColor(renderer->sdl_renderer, 255, 0, 0, 255);
        if (seg.portal)
            SDL_SetRenderDrawColor(renderer->sdl_renderer, 0, 255, 255, 255);
        vec2 *verts[2];
        get_vertices(&g_state->level, &seg, verts);

        vec2 wv1 = editor_world_to_screen(estate, *verts[0]);
        vec2 wv2 = editor_world_to_screen(estate, *verts[1]);
        SDL_RenderLine(renderer->sdl_renderer, wv1[0], wv1[1], wv2[0], wv2[1]);
        vec2 normal = get_segment_normal(&g_state->level, &seg);
        vec2 nstart = wv1 + (wv2 - wv1) / 2.0;
        SDL_RenderLine(renderer->sdl_renderer, nstart[0], nstart[1], (nstart + normal * 5.0f)[0], (nstart + normal * 5.0f)[1]);
    }

    SDL_SetRenderDrawColor(renderer->sdl_renderer, 250, 245, 200, 255);
    float size = 5.0f; // taille en pixels monde
    h_iter_t vertiter = h_array_iter(&g_state->level.vertices);
    H_FOREACH(vec2, vert, vertiter) {
        vec2 wv = editor_world_to_screen(estate, vert);
        SDL_FRect r = {
            wv[0] - size * 0.5f,
            wv[1] - size * 0.5f,
            size,
            size
        };

        SDL_RenderFillRect(renderer->sdl_renderer, &r);
    }

    SDL_SetRenderDrawColor(renderer->sdl_renderer, 0, 255, 0, 255);
    h_iter_t seciter = h_array_iter(&g_state->level.sections);
    H_FOREACH_PTR(sector_t, sec, seciter) {
        vec2 wc = editor_world_to_screen(estate, get_section_center(&g_state->level, sec));
        SDL_FRect r = {
            wc[0] - size * 0.5f,
            wc[1] - size * 0.5f,
            size,
            size
        };
        SDL_RenderFillRect(renderer->sdl_renderer, &r);
    }

    h_iter_t transform_iter = component_pool_iter(&g_state->level.world, TRANSFORM);
    SDL_SetRenderDrawColor(renderer->sdl_renderer, 255, 255, 0, 255);
    H_FOREACH(transform_t, t, transform_iter) {
        vec2 wv = editor_world_to_screen(estate, t.position);
        size = 4.0f;
        SDL_FRect r = {
            wv[0] - size * 0.5f,
            wv[1] - size * 0.5f,
            size,
            size
        };
        SDL_RenderFillRect(renderer->sdl_renderer, &r);
    }

    SDL_SetRenderDrawColor(renderer->sdl_renderer, 255, 255, 255, 255);
    SDL_RenderPresent(renderer->sdl_renderer);
}

void editor_event_handler(void *state, SDL_Event *event) {
    editor_layer_state_t *estate = (editor_layer_state_t *)state;

    if (event->type == SDL_EVENT_KEY_DOWN) {
        switch (event->key.key) {
            case SDLK_UP :
                estate->movement[1] = -1.0;
                break;
            case SDLK_DOWN :
                estate->movement[1] = 1.0;
                break;
            case SDLK_LEFT:
                estate->movement[0] = -1.0;
                break;
            case SDLK_RIGHT :
                estate->movement[0] = 1.0;
                break;
        }
    }
    if (event->type == SDL_EVENT_KEY_UP) {
        switch (event->key.key) {
            case SDLK_UP :
            case SDLK_DOWN :
                estate->movement[1] = 0.0;
                break;
            case SDLK_LEFT:
            case SDLK_RIGHT :
                estate->movement[0] = 0.0;
                break;
        }
    }

    if (estate->movement[0] != 0 || estate->movement[1] != 0)
        estate->movement = normalize(estate->movement);
}
