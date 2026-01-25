#include "../core/raycast_renderer.h"

/*

    provides rendering function for game and 3d editor layers.
    TODO : Implement billboards rendering

 */

#include <float.h>

#include "app.h"
#include "ecs/components/camera.h"
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

    if (!(seg->texid+1)) {
        draw_vert_line(renderer, x, y0 ,y1, 0XFF00FFFF);
        return;
    }

    vec2 *verts[2];
    get_vertices(&g_state->level, seg, verts);

    vec2 relint = inter - *verts[0];
    float t = magnitude(relint) / magnitude(*verts[1] - *verts[0]);
    float uvx = lerp(seg->uvs[1], seg->uvs[0], t);
    float _intpt;
    uvx = modff(uvx, &_intpt);

    for (i32 y = y0; y <= y1; ++y) {
        float v_factor = (y - y0) / h;
        float uvy = lerp(seg->uvs[3], seg->uvs[2],v_factor);

        uvy = modff(uvy, &_intpt);

        i32 texcoord_x = (i32)(uvx * (g_texture_atlas[seg->texid].surface->w - 1));
        i32 texcoord_y = (i32)(uvy * (g_texture_atlas[seg->texid].surface->h - 1));

        i32 idx = y * renderer->w + x;
        if (idx < 0 || idx >= renderer->w * renderer->h) continue;

        u32 color = get_pixel(&g_texture_atlas[seg->texid], (vec2){texcoord_x, texcoord_y});
        renderer->pixels[idx] = color;
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
void render_floor_ceil(renderer_t *renderer, i32 x, i32 y0, i32 y1, sector_t *sec, vec2 raydir, float floordiff, float ceildiff, vec2 camfwd) {
    vec2 camright = (vec2){-camfwd[1], camfwd[0]};

    float cosbeta = fabsf(dot(camfwd, raydir));
    float invCos = 1.0f / cosbeta;

    // floor
    if (!sec->fctexid[0])
        draw_vert_line(renderer, x, 0 ,y0, 0xFF00FFFF);
    else {
        float floorz = floordiff - EYE_Z;

        const vec2 surface_size = {
            g_texture_atlas[sec->fctexid[0]].surface->w - 1,
            g_texture_atlas[sec->fctexid[0]].surface->h - 1
        };

        // textured floor
        for (i32 y = 0; y < y0; ++y) {
            float r = (float)y - renderer->h * 0.5f;
            float d = (floorz * ((float)renderer->h)) / r * invCos;

            vec2 wpos = g_camera->transform->position + d * raydir;

            vec2 uvs = (vec2){
                frac(wpos[0]*sec->fcuvs[0]),
                frac(wpos[1]*sec->fcuvs[1])
            };

            vec2 texcoord = uvs * surface_size;

            i32 idx = y * renderer->w + x;
            if (idx >= renderer->w * renderer->h) continue;
            if (idx <0) continue;

            renderer->pixels[idx] = get_pixel(&g_texture_atlas[sec->fctexid[0]], texcoord);
            renderer->depth_buffer[idx] = (_Float16)d;
        }
    }

    //ceil
    if (!sec->fctexid[1])
        draw_vert_line(renderer, x, y1 ,renderer->h, 0xFF00FFFF);
    else {
        float ceilz = ceildiff + EYE_Z;

        const vec2 surface_size = {
            g_texture_atlas[sec->fctexid[1]].surface->w - 1,
            g_texture_atlas[sec->fctexid[1]].surface->h - 1
        };

        // textured ceiling
        for (i32 y = y1; y < renderer->h; ++y) {
            float r = (f32)y - renderer->h * 0.5f;
            float d = (ceilz * ((f32)renderer->h)) / r * invCos;

            vec2 wpos = g_camera->transform->position + d * raydir;
            vec2 uvs = (vec2){
                frac(wpos[0]*sec->fcuvs[2]),
                frac(wpos[1]*sec->fcuvs[3])
            };

            vec2 texcoord = uvs * surface_size;

            i32 idx = y * renderer->w + x;
            if (idx >= renderer->w * renderer->h) continue;
            if (idx < 0) continue;
            u32 color = get_pixel(&g_texture_atlas[sec->fctexid[1]], texcoord);
            renderer->pixels[idx] = color;
            renderer->depth_buffer[idx] = (_Float16)d;
        }
    }
}

void render_billboarded_sprite(renderer_t *renderer, vec2 pos, vec2 size, texture_t *tex, sector_t *sec, f32 posz) {
    const vec2 camfwd = camera_forward(g_camera);
    const vec2 viewpos = pos - g_camera->transform->position;
    if (dot(camfwd, viewpos) < 0) return;
    const vec2 camright = (vec2){-camfwd[1], camfwd[0]};
    const i32 width = tex->surface->w;
    const i32 height = tex->surface->h;
    const vec2 dir = normalize(viewpos);
    const vec2 plane = {0, tanf(g_camera->fov)};
    const f32 d = magnitude(viewpos) * fabsf(dot(camfwd, dir));

    f64 invDet = 1.0 / cross2(camright, camfwd);
    dvec2 transform = invDet * (dvec2){
        (f64)cross2(viewpos, camfwd),
        -(f64)cross2(viewpos, -camright)
    };

    sector_t *current_sec = h_array_get( &g_state->level.sections, g_camera->sector->sector );
    if (!sec) return;
    f32 dfloor =  sec->floor - (current_sec->floor + EYE_Z);

    i32 screenpos_z = (int)((renderer->w / 2.0) * (f32)(dfloor + posz) / transform[1]);

    i32 screenpos_x = (int)((renderer->w / 2.0) * (1 + transform[0] / transform[1]));

    i32 screenheight = abs((int)(renderer->h / transform[1])) * size[0];
    i32 screenwidth = abs((int)(renderer->h / transform[1])) * size[1];

    vec2 drawstart = {
        -screenwidth,
        -screenheight
    };
    drawstart = drawstart / 2 + (vec2){screenpos_x, renderer->h *0.5 - screenpos_z};
    vec2 drawstart_c = {CLAMP(drawstart[0], 0, renderer->w), CLAMP(drawstart[1], 0, renderer->h)};

    vec2 drawend = {
        screenwidth,
        screenheight
    };
    drawend = drawend / 2 + (vec2){screenpos_x, renderer->h / 2 - screenpos_z};
    vec2 drawend_c = {CLAMP(drawend[0], 0, renderer->w), CLAMP(drawend[1], 0, renderer->h)};

    vec2 drawsize = drawend - drawstart;
    vec2 invSize = (vec2){1, 1}/drawsize;

    vec2 drawsize_c = drawend_c - drawstart_c;

    if (drawsize_c[0] <= 0 || drawsize_c[1] <= 0) return;

    for (i32 y = drawstart_c[1]; y <= drawend_c[1]; ++y) {
        for (i32 x = drawstart_c[0]; x <= drawend_c[0]; ++x) {
            if (x < 0 || x >= renderer->w || y < 0 || y >= renderer->h) continue;
            i32 idx = y * renderer->w + x;
            if (d >= renderer->depth_buffer[idx]) continue;

            vec2 uv = {x, y};
            uv = (uv - drawstart) * invSize;
            uv[1] = 1 - uv[1];
            if (uv[0] > 1 || uv[1] > 1) continue;
            vec2 texcoord = uv * (vec2){tex->surface->w - 1, tex->surface->h - 1};
            u32 px = get_pixel(tex, texcoord);
            if (!(px&0x000000FF)) continue;
            renderer->depth_buffer[idx] = d;
            renderer->pixels[idx] = px;
        }
    }

}

/*
 *      RAYCASTING RENDERER FUNCTION
 */

/**
 * Renders a 3D scene from the perspective of a camera using a raycasting technique.
 * The method performs wall segment rendering, portal traversal, lighting, and draws
 * floors, ceilings, and debug information.
 *
 * @param state Pointer to the layer state data structure.
 * @param renderer Pointer to the rendering context object responsible for managing screen dimensions, textures, and pixel buffers.
 */
void raycaster_render(void *state, struct renderer_s *renderer) {
    const f32 k = renderer->h;
    const sector_t *current_sec = h_array_get( &g_state->level.sections, g_camera->sector->sector  );
    const vec2 camfwd = camera_forward(g_camera);

    // for ray rotation
    float sinA = sin(g_camera->transform->rotation);
    float cosA = cos(g_camera->transform->rotation);

    struct segstack_el_s {
        segment_t *seg;
        sector_t *sec;
        float d;
        vec2 inter;
    };
    //h_array_t segstack = H_CREATE_ARRAY(struct segstack_el_s, 8); // stack of segment to render through portals for a single line
    struct segstack_el_s segstack[SEGMENT_STACK_CAPACITY];
    size_t segstack_size = 0;

    for (int x=0;x<renderer->w;x += 1) {
        const sector_t *sec = current_sec;
        vec2 raydir = (vec2){
            renderer->ray_directions[x][0] * cosA - renderer->ray_directions[x][1] * sinA,
            renderer->ray_directions[x][0] * sinA + renderer->ray_directions[x][1] * cosA
        };
        vec2 rayorigin = g_camera->transform->position;
        const float dist_correction_factor = fabsf(dot(camfwd, raydir));

        vec2 inter = (vec2){NAN, NAN};
        float td = 0.0;

        while (sec) {
            segment_t *segment = NULL;
            float d = FLT_MAX;
            // for each segment in the current sector
            for (segment_t *seg = sec->first_segment; seg < sec->first_segment + sec->num_segments; seg++) {
                vec2 *verts[2];
                get_vertices(&g_state->level, seg, verts);
                vec2 norm = get_segment_normal(&g_state->level, seg);
                // backface culling
                if (dot(norm, raydir) > 0) continue;
                // frustum culling
                if (dot(*verts[0] - rayorigin, raydir) <= 0 &&
                    dot(*verts[1] - rayorigin, raydir) <= 0) continue;

                vec2 temp_inter = intersect_seg(rayorigin, rayorigin + raydir * 1000, *verts[0], *verts[1]);
                if (isnan(temp_inter[0])) continue;
                float dt = magnitude(temp_inter - rayorigin) * dist_correction_factor;
                if (dt < 0 || dt >= d) continue;
                d = dt;
                inter = temp_inter;
                segment = seg;
            }
            if (!segment) break;
            td += d;
            segstack[segstack_size++] = (struct segstack_el_s){segment, sec, td, inter};
            if (segstack_size>=SEGMENT_STACK_CAPACITY) break;
            if (segment->portal) {
                sec = h_array_get( &g_state->level.sections, segment->portal - 1);
                rayorigin = inter;
                continue;
            }
            sec = NULL;
            break;
        }

        if (isnan(inter[0])) continue;
        while (segstack_size > 0) {
            const struct segstack_el_s el = segstack[segstack_size - 1];
            const f32 projection_factor = k/el.d;

            if (el.d <= 0) {
                segstack_size--;
                continue;
            }

            vec2 yspan = {renderer->h/2, renderer->h/2};
            vec2 addi = {0};
            addi = (vec2) {
                - (current_sec->floor - el.sec->floor + EYE_Z),
                (el.sec->ceil - current_sec->floor + EYE_Z)
            };
            yspan = yspan + addi*projection_factor;

            float wall_px_height = yspan[1] - yspan[0];
            if (el.seg->portal) {
                sector_t *portal_sec = h_array_get( &g_state->level.sections, el.seg->portal - 1);

                f32 dfloor =  portal_sec->floor - el.sec->floor;
                dfloor = dfloor > 0 ? dfloor : 0;

                f32 dceil = el.sec->ceil - portal_sec->ceil;
                dceil = dceil > 0 ? dceil : 0;

                vec2 dfloor_ceil = (vec2){dfloor, -dceil} * projection_factor;
                vec2 portal_yspan = yspan + dfloor_ceil;

                render_segment_line(renderer, x, yspan[0] ,portal_yspan[0], el.seg, el.inter, wall_px_height, el.d);
                render_segment_line(renderer, x, portal_yspan[1] ,yspan[1], el.seg, el.inter, wall_px_height, el.d);

            }
            else
                render_segment_line(renderer, x, yspan[0] ,yspan[1], el.seg, el.inter, wall_px_height, el.d);

            float dfloor = el.sec->floor - current_sec->floor;
            float dceil = el.sec->ceil - current_sec->floor;

            render_floor_ceil(renderer, x, yspan[0], yspan[1], el.sec, raydir, dfloor, dceil, camfwd);

            segstack_size--;
        }
    }

    entity_query_t billboard_query = query_entities(&g_state->level.world, 3, (component_id_t[]){TRANSFORM, SPRITE, BILLBOARD});

    h_iter_t billboard_iter = query_entities_iter(&billboard_query);
    H_FOREACH(entity_t, e, billboard_iter) {
        transform_t *t = get_component(&g_state->level.world, e, TRANSFORM);
        sprite_t *s = get_component(&g_state->level.world, e, SPRITE);
        billboard_t *b = get_component(&g_state->level.world, e, BILLBOARD);
        render_billboarded_sprite(renderer, t->position, t->scale, &g_texture_atlas[s->atlas_index], h_array_get(&g_state->level.sections, b->section), b->z);
    }


    SDL_UpdateTexture(renderer->render_texture, nullptr, renderer->pixels, renderer->w * sizeof(u32));
    SDL_RenderTextureRotated(
      renderer->sdl_renderer, renderer->render_texture, nullptr, nullptr, 0, nullptr, SDL_FLIP_VERTICAL
    );

    char fps_debug[16];
    sprintf(fps_debug, "FPS : %.2f", 1000.0f / fmaxf(clock_delta(&g_state->deltaclock), 0.1));
    SDL_RenderDebugText(renderer->sdl_renderer, 5, 5,fps_debug);

    char level_path_debug[128];
    sprintf(level_path_debug, "LEVEL : %s", g_state->level.level_name);
    SDL_RenderDebugText(renderer->sdl_renderer, 5, 20, level_path_debug);

    char current_sector_debug[32];
    sprintf(current_sector_debug, "CURRENT SECTOR : %d", g_camera->sector->sector);
    SDL_RenderDebugText(renderer->sdl_renderer, 5, 35, current_sector_debug);

    SDL_RenderPresent(renderer->sdl_renderer);
}
