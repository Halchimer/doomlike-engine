#include "level_editor_layer.h"

#include "../core/app.h"
#include "../core/renderer.h"
#include "../editor/level_editor_panels.h"
#include "SDL3/SDL_render.h"

// TODO: Implement a level editor using nuklear as a GUI lib

void editor_gui(void *state);
void validate_selection(void *state);

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

    estate->prev_pos = (vec2){NAN, NAN};
    estate->sector = -1;
    estate->selection = H_CREATE_ARRAY(void*, 64);
}

void editor_layer_resume(void *state) {
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
        SDL_SetRenderDrawColor(renderer->sdl_renderer, EDITOR_SEGMENT_COLOR);
        if (seg.portal)
            SDL_SetRenderDrawColor(renderer->sdl_renderer, EDITOR_SEGMENT_PORTAL_COLOR);
        vec2 *verts[2];
        get_vertices(&g_state->level, &seg, verts);

        vec2 wv1 = editor_world_to_screen(estate, *verts[0]);
        vec2 wv2 = editor_world_to_screen(estate, *verts[1]);
        SDL_RenderLine(renderer->sdl_renderer, wv1[0], wv1[1], wv2[0], wv2[1]);
        vec2 normal = get_segment_normal(&g_state->level, &seg);
        vec2 nstart = wv1 + (wv2 - wv1) / 2.0;
        SDL_RenderLine(renderer->sdl_renderer, nstart[0], nstart[1], (nstart + normal * 5.0f)[0], (nstart + normal * 5.0f)[1]);
    }

    float size = 5.0f;
    h_iter_t vertiter = h_array_iter(&g_state->level.vertices);
    H_FOREACH_PTR(vec2, vert, vertiter) {
        SDL_SetRenderDrawColor(renderer->sdl_renderer, EDITOR_VERTICE_COLOR);
        if (estate->edit_mode==EDIT_MODE_VERTICES || estate->edit_mode==EDIT_MODE_ALL) {
            h_iter_t seliter = h_array_iter(&estate->selection);

            typedef vec2* vec2_ptr;
            H_FOREACH(vec2_ptr, vsel, seliter) {
                if (vert != vsel) continue;
                SDL_SetRenderDrawColor(renderer->sdl_renderer, EDITOR_VERTICE_COLOR_SELECTED);
                break;
            }
        }

        vec2 wv = editor_world_to_screen(estate, *vert);
        SDL_FRect r = {
            wv[0] - size * 0.5f,
            wv[1] - size * 0.5f,
            size,
            size
        };

        SDL_RenderFillRect(renderer->sdl_renderer, &r);
    }

    h_iter_t seciter = h_array_iter(&g_state->level.sections);
    H_FOREACH_PTR(sector_t, sec, seciter) {
        SDL_SetRenderDrawColor(renderer->sdl_renderer, EDITOR_SECTOR_COLOR);

        if (estate->sector >=0 && sec == h_array_get(&g_state->level.sections,estate->sector)) {
            vec4 bounds = sec->bounds;
            vec2 bpos = editor_world_to_screen(state, H_SWIZZLE(bounds, 0,1));
            vec2 bsize = editor_world_to_screen(state, H_SWIZZLE(bounds, 2,3)) + estate->pos;
            SDL_FRect r = {
                bpos[0]-EDITOR_SECTOR_BOUNDS_MARGIN,
                bpos[1]-EDITOR_SECTOR_BOUNDS_MARGIN,
                bsize[0]+2.0*EDITOR_SECTOR_BOUNDS_MARGIN,
                bsize[1]+2.0*EDITOR_SECTOR_BOUNDS_MARGIN
            };
            SDL_SetRenderDrawColor(renderer->sdl_renderer, EDITOR_SECTOR_COLOR_SELECTED);
            SDL_RenderRect(renderer->sdl_renderer, &r);
        }

        vec2 wc = editor_world_to_screen(estate, get_section_center(&g_state->level, sec));
        char buf[8];
        sprintf(buf, "%d", (int)(sec - (sector_t *)g_state->level.sections.data));
        SDL_RenderDebugText(renderer->sdl_renderer, wc[0], wc[1], buf);
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

    // render selection bounds

    float mx, my;
    SDL_MouseButtonFlags mb = SDL_GetMouseState(&mx, &my);
    if (mb & SDL_BUTTON_LEFT) {
        vec2 mpos = (vec2){mx, my};
        vec2 diff = mpos - estate->prev_pos;
        if (magnitude(diff) <= 0.01) goto draw_selection_bound_end;
        SDL_FRect r = {
            .x = estate->prev_pos[0],
            .y = estate->prev_pos[1],
            .w = diff[0],
            .h = diff[1]
        };
        SDL_RenderRect(renderer->sdl_renderer, &r);
        draw_selection_bound_end:;
    }

    editor_gui(state);
}

// event handler, for editor shortcuts
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

    // mouse events for selection.

    struct nk_context *ctx = g_nuklear_instance->ctx;
    bool shouldProcessMouse = !nk_window_is_any_hovered(ctx);
    if (shouldProcessMouse && event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        switch (event->button.button) {
            case SDL_BUTTON_LEFT:
                if (isnan(estate->prev_pos[0])) {
                    float x, y;
                    SDL_GetMouseState(&x, &y);
                    estate->prev_pos = (vec2){x, y};
                }
                break;
            default:break;
        }
    }
    else if (shouldProcessMouse && event->type == SDL_EVENT_MOUSE_BUTTON_UP) {
        switch (event->button.button) {
            case SDL_BUTTON_LEFT:
                validate_selection(state);
                break;
            default :break;
        }
    }
}

// editor gui function
void editor_gui(void *state) {
    editor_layer_state_t *estate = (editor_layer_state_t *)state;
    struct nk_context *ctx = g_nuklear_instance->ctx;
    struct nk_colorf bg = g_nuklear_instance->bg;

    /* GUI */
    if (nk_begin(ctx, "LEVEL EDITOR", nk_rect(50, 50, 230, 250),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
        NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
    {
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_property_int(ctx, "Editing Sector", -1, &estate->sector, g_state->level.sections.size-1, 1, 1);

        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "Edit Mode : ", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 20, 3);
        if (nk_option_label(ctx, "Vertices", estate->edit_mode==EDIT_MODE_VERTICES)) estate->edit_mode = EDIT_MODE_VERTICES;
        if (nk_option_label(ctx, "Segments", estate->edit_mode==EDIT_MODE_SEGMENTS)) estate->edit_mode = EDIT_MODE_SEGMENTS;
        if (nk_option_label(ctx, "Both", estate->edit_mode==EDIT_MODE_ALL)) estate->edit_mode = EDIT_MODE_ALL;

        if (nk_tree_push(ctx, NK_TREE_TAB, "Tools", NK_MINIMIZED)) {

            nk_button_label(ctx, "Save As");
            nk_button_label(ctx, "Save");
            if (nk_button_label(ctx, "Create Vertex")) {
                const vec2 z = {0,0};
                vec2 *v = H_ARRAY_PUSH(vec2, g_state->level.vertices, z);
                estate->selection.size = 0;
                H_ARRAY_PUSH(vec2*, estate->selection,v);
            }

            nk_tree_pop(ctx);
        }
    }
    nk_end(ctx);

    if (estate->sector >=0) {
        sector_t *sector = h_array_get(&g_state->level.sections, estate->sector);

        draw_sector_edit_panel(sector);
    }

    if (estate->selection.size == 1 && estate->edit_mode == EDIT_MODE_VERTICES)
        draw_vertex_edit_panel(H_ARRAY_GET(vec2*, estate->selection, 0));
}
void validate_selection(void *state) {
    editor_layer_state_t *estate = (editor_layer_state_t *)state;

    if (isnan(estate->prev_pos[0])) return;

    float mx, my;
    SDL_GetMouseState(&mx, &my);
    vec2 mouse_pos = {mx, my};
    vec2 sel_bound_start = VMIN2(mouse_pos, estate->prev_pos);
    vec2 sel_bound_end = VMAX2(mouse_pos, estate->prev_pos);
    vec2 sel_bound_size = sel_bound_end - sel_bound_start;
    vec4 sel_bounds = {sel_bound_start[0], sel_bound_start[1], sel_bound_size[0], sel_bound_size[1]};

    estate->selection.size=0;

    h_iter_t viter = h_array_iter(&g_state->level.vertices);
    H_FOREACH_PTR(vec2, vert, viter) {
        vec2 svert = editor_world_to_screen(state, *vert);
        if (point_in_bounds(svert, sel_bounds))
            H_ARRAY_PUSH(vec2*,estate->selection,vert);
    }

    estate->prev_pos = (vec2){NAN, NAN};
}