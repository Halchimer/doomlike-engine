#include "../core/raycast_renderer.h"

/*

    provides rendering function for game and 3d editor layers.
    TODO : Implement billboards rendering

 */

#include <float.h>

#include "app.h"
#include "camera.h"
#include "../core/renderer.h"
#include "../core/texture_atlas.h"
#include "../utils/utils.h"

/**
 * Renders a vertical segment line on the renderer, applying texture mapping and lighting effects if a texture is available.
 *
 * @param renderer Pointer to the rendering context object.
 * @param x The horizontal position on the screen where the vertical line will be drawn.
 * @param y0 The starting vertical position of the line.
 * @param y1 The ending vertical position of the line.
 * @param seg Pointer to the segment structure representing the line segment to render.
 * @param inter Intersection point in world space relative to the segment.
 * @param h The height of the segment in screen space.
 * @param d The distance from the camera to the segment being rendered.
 */
void render_segment_line(renderer_t *renderer, i32 x, i32 y0, i32 y1, segment_t *seg, vec2 inter, f32 h, f32 d) {
    if (y1 - y0 <= 0) return;

    draw_vert_line_depth_buffer(renderer, x, y0 ,y1, (_Float16)d);

    if (!seg->texid) {
        draw_vert_line(renderer, x, y0 ,y1, 0XFF00FFFF);
        return;
    }

    vec2 relint = inter - *seg->vertices[0];
    float t = magnitude(relint) / magnitude(*seg->vertices[1] - *seg->vertices[0]);
    float uvx = lerp(seg->uvs[1], seg->uvs[0], t);
    float _intpt;
    uvx = modff(uvx, &_intpt);

    for (i32 y = y0; y <= y1; ++y) {
        float v_factor = (y - y0) / h;
        float uvy = lerp(seg->uvs[3], seg->uvs[2],v_factor);

        uvy = modff(uvy, &_intpt);

        i32 texcoord_x = (i32)(uvx * (g_texture_atlas[seg->texid - 1].surface->w - 1));
        i32 texcoord_y = (i32)(uvy * (g_texture_atlas[seg->texid - 1].surface->h - 1));

        i32 idx = y * renderer->w + x;
        if (idx < 0 || idx >= renderer->w * renderer->h) continue;

        vec4 color = hex_to_vec4(get_pixel(&g_texture_atlas[seg->texid - 1], (vec2){texcoord_x, texcoord_y}));
        vec2 norm = get_segment_normal(seg);
        vec2 sun = (vec2){1, 0};
        sun = normalize(sun);
        float dotprod = fabsf(dot(norm, sun));
        dotprod = CLAMP(dotprod, 0.5f, 1.0f);
        vec4 shade = {dotprod, dotprod, dotprod, 1.0f};
        color = color * shade;

        renderer->pixels[idx] = vec4_to_hex(color);
    }
}

/**
 * Renders the floor and ceiling textures or solid colors for a specific vertical slice of the screen based on
 * the current rendering context, camera view direction, and section data.
 *
 * @param renderer Pointer to the rendering context object.
 * @param x The horizontal position on the screen corresponding to the current vertical slice being rendered.
 * @param y0 The starting vertical screen coordinate of the floor texture or solid color.
 * @param y1 The ending vertical screen coordinate of the ceiling texture or solid color.
 * @param sec Pointer to the section structure containing information about floor and ceiling textures or colors.
 * @param raydir Normalized direction of the ray for this vertical screen slice in world space.
 * @param floordiff Distance difference between the current sector's floor level and the floor height of the section.
 * @param ceildiff Distance difference between the current sector's floor level and the ceiling height of the section.
 */
void render_floor_ceil(renderer_t *renderer, i32 x, i32 y0, i32 y1, section_t *sec, vec2 raydir, float floordiff, float ceildiff) {
    vec2 camfwd = camera_forward(g_camera);
    vec2 camright = (vec2){-camfwd[1], camfwd[0]};

    float cosbeta = fabsf(dot(camfwd, raydir));
    float cosalpha = fabsf(dot(camright, raydir));
    float sinalpha = sin(acosf(cosalpha));

    raydir = normalize(raydir);

    // floor
    if (!sec->fctexid[0])
        draw_vert_line(renderer, x, 0 ,y0, 0xFF00FFFF);
    else {
        // textured floor
        for (i32 y = 0; y < y0; ++y) {
            float r = (f32)y - ((f32)renderer->h/2);
            if (fabsf(r) < 0.001f) r = copysignf(0.001f, r);
            float s = (floordiff - EYE_Z) * (renderer->h) / r;
            float d = s / cosbeta;
            vec2 pxwpos = g_camera->position + d * raydir;
            vec2 uvs = (vec2){
                frac(pxwpos[0]*sec->fcuvs[0]),
                frac(pxwpos[1]*sec->fcuvs[1])
            };

            vec2 texcoord = uvs * (vec2){g_texture_atlas[sec->fctexid[0] - 1].surface->w - 1, g_texture_atlas[sec->fctexid[0] - 1].surface->h - 1};

            i32 idx = y * renderer->w + x;
            if (idx >= renderer->w * renderer->h) continue;
            renderer->pixels[idx] = get_pixel(&g_texture_atlas[sec->fctexid[0] - 1], texcoord);
            renderer->depth_buffer[idx] = (_Float16)d;
        }
    }

    //ceil
    draw_vert_line(renderer, x, y1 ,renderer->h, 0xFF00FFFF);

    if (!sec->fctexid[1])
        draw_vert_line(renderer, x, y1 ,renderer->h, 0xFF00FFFF);
    else {
        // textured ceiling
        for (i32 y = y1; y < renderer->h; ++y) {
            float r = (f32)y - ((f32)renderer->h / 2.0);
            if (fabsf(r) < 0.001f) r = copysignf(0.001f, r);

            float s = ((ceildiff + EYE_Z) * ((f32)renderer->h)) / r;
            float d =  s / cosbeta;

            vec2 pxwpos = g_camera->position + d * raydir;
            vec2 uvs = (vec2){
                frac(pxwpos[0]*sec->fcuvs[2]),
                frac(pxwpos[1]*sec->fcuvs[3])
            };

            vec2 texcoord = uvs * (vec2){g_texture_atlas[sec->fctexid[1] - 1].surface->w - 1, g_texture_atlas[sec->fctexid[0] - 1].surface->h - 1};

            i32 idx = y * renderer->w + x;
            if (idx >= renderer->w * renderer->h) continue;
            vec4 color = hex_to_vec4(get_pixel(&g_texture_atlas[sec->fctexid[1] - 1], texcoord));
            color = color * 0.5f;
            color[3] = 1.0f;
            renderer->pixels[idx] = vec4_to_hex(color);
            renderer->depth_buffer[idx] = (_Float16)d;
        }
    }
}

/**
 * Renders a 3D scene from the perspective of a camera using a raycasting technique.
 * The method performs wall segment rendering, portal traversal, lighting, and draws
 * floors, ceilings, and debug information.
 *
 * @param state Pointer to the layer state data structure.
 * @param renderer Pointer to the rendering context object responsible for managing screen dimensions, textures, and pixel buffers.
 */
void raycaster_render(void *state, struct renderer_s *renderer) {
    fill_pixels(renderer, 0x000000FF);
    f32 k = renderer->h;
    section_t *current_sec = h_array_get( &g_state->level.sections, g_camera->sector );
    struct segstack_el_s {
        segment_t *seg;
        section_t *sec;
        float d;
        vec2 inter;
    };
    h_array_t segstack = H_CREATE_ARRAY(struct segstack_el_s, 8); // stack of segment to render through portals for a single line

    for (int x=0;x<renderer->w;x += 1) {
        section_t *sec = current_sec;
        vec2 raydir = camera_raydir(g_camera, (float)x/(float)renderer->w);
        vec2 rayorigin = g_camera->position;

        vec2 inter = (vec2){NAN, NAN};

        float td = 0.0;

        while (sec) {
            segment_t *segment = NULL;
            float d = FLT_MAX;
            for (segment_t *seg = sec->first_segment; seg < sec->first_segment + sec->num_segments; seg++) {

                vec2 norm = get_segment_normal(seg);
                // backface culling
                if (dot(norm, raydir) > 0) continue;
                // frustum culling
                if (dot(*seg->vertices[0] - rayorigin, raydir) <= 0 &&
                    dot(*seg->vertices[1] - rayorigin, raydir) <= 0) continue;

                vec2 temp_inter = intersect_seg(rayorigin, rayorigin + raydir * 1000, *seg->vertices[0], *seg->vertices[1]);
                if (isnan(temp_inter[0])) continue;
                float dt = magnitude(temp_inter - rayorigin) * fabsf(dot(camera_forward(g_camera), raydir));
                if (dt >= d || dt <= 0) continue;
                d = dt;
                inter = temp_inter;
                segment = seg;
            }
            if (!segment) break;
            td += d;
            struct segstack_el_s el = {segment, sec, td, inter};
            H_ARRAY_PUSH(struct segstack_el_s, segstack, el);
            if (segment->portal) {
                sec = h_array_get( &g_state->level.sections, segment->portal - 1);
                rayorigin = inter;
                continue;
            }
            sec = NULL;
            break;
        }

        if (isnan(inter[0])) continue;
        while (segstack.size > 0) {
            struct segstack_el_s el = H_ARRAY_GET(struct segstack_el_s, segstack, segstack.size - 1);

            if (el.d <= 0) {
                segstack.size--;
                continue;
            }
            vec2 w = *el.seg->vertices[0] + *el.seg->vertices[1];
            u32 seed = h_pcg_hash((h_pcg_hash(w[0]) + h_pcg_hash(w[1])));
            u32 color = ( (h_randi(seed, 0, 0xFFFFFF)) << 8) | 0xFF ;

            f32 y0, y1;
            if (el.sec == current_sec) {
                y0 = renderer->h/2 - ((EYE_Z)/el.d)*k;
                y1 = renderer->h/2 + ((current_sec->ceil - current_sec->floor + EYE_Z)/el.d)*k;
            }
            else {

                y0 = renderer->h/2 - ((current_sec->floor - el.sec->floor + EYE_Z)/el.d)*k;
                y1 = renderer->h/2 + ((el.sec->ceil - current_sec->floor + EYE_Z)/el.d)*k;
            }

            if (el.seg->portal) {
                section_t *portal_sec = h_array_get( &g_state->level.sections, el.seg->portal - 1);

                f32 dfloor =  portal_sec->floor - el.sec->floor;
                dfloor = dfloor > 0 ? dfloor : 0;
                dfloor = (dfloor/el.d)*k;

                f32 dceil = el.sec->ceil - portal_sec->ceil;
                dceil = dceil > 0 ? dceil : 0;
                dceil = (dceil/el.d)*k;

                render_segment_line(renderer, x, y0 ,y0 + dfloor, el.seg, el.inter, y1 - y0, el.d);
                render_segment_line(renderer, x, y1 - dceil ,y1, el.seg, el.inter, y1 - y0, el.d);

            }
            else
                render_segment_line(renderer, x, y0 ,y1, el.seg, el.inter, y1 - y0, el.d);

            float dfloor = el.sec->floor - current_sec->floor;
            float dceil = el.sec->ceil - current_sec->floor;

            render_floor_ceil(renderer, x, y0, y1, el.sec, raydir, dfloor, dceil);

            segstack.size--;
        }
    }

    SDL_UpdateTexture(renderer->render_texture, nullptr, renderer->pixels, renderer->w * sizeof(u32));
    SDL_RenderTextureRotated(
      renderer->sdl_renderer, renderer->render_texture, nullptr, nullptr, 0, nullptr, SDL_FLIP_VERTICAL
    );

    char fps_debug[16];
    sprintf(fps_debug, "FPS : %.2f", 1000.0f / clock_delta(&g_state->deltaclock));
    SDL_RenderDebugText(renderer->sdl_renderer, 5, 5,fps_debug);

    char level_path_debug[128];
    sprintf(level_path_debug, "LEVEL : %s", g_state->level.level_name);
    SDL_RenderDebugText(renderer->sdl_renderer, 5, 20, level_path_debug);

    char current_sector_debug[32];
    sprintf(current_sector_debug, "CURRENT SECTOR : %d", g_camera->sector);
    SDL_RenderDebugText(renderer->sdl_renderer, 5, 35, current_sector_debug);

    SDL_RenderPresent(renderer->sdl_renderer);
}
