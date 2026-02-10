#include "lightmap.h"

#include "../utils/utils.h"
#include "../core/app.h"
#include "../core/level.h"

vec4 radiance_at_world_position(vec4 position, vec4 norm, sector_t *sec) {
    vec4 radiance = {0, 0, 0, 1.0};

    entity_query_t lquery = query_entities(&g_state->level.world, 3, (component_id_t[]){TRANSFORM, LIGHT, SECTOR_COMPONENT});
    h_iter_t liter = query_entities_iter(&lquery);
    radiance[3] = 1.0F;
    H_FOREACH(entity_t, e, liter) {
        light_t *l = get_component(&g_state->level.world, e, LIGHT);
        transform_t *t = get_component(&g_state->level.world, e, TRANSFORM);
        sector_component_t *sc = get_component(&g_state->level.world, e, SECTOR_COMPONENT);
        sector_t *light_sector = h_array_get(&g_state->level.sections, sc->sector);

        vec4 wpos = H_SWIZZLE(t->position, 0, 1, 0, 0);
        wpos[2] = light_sector->floor + l->height;
        wpos[3] = 0.0;

        vec4 p_to_light = wpos - position;

        vec2 inter = {NAN, NAN};

        vec4 raypos = position + norm * 0.005f;
        sector_t *current = sec;
        while (current) {
            bool intersection = false;
            for (segment_t *seg = current->first_segment; seg < current->first_segment + current->num_segments; ++seg) {
                vec2 seg_norm = get_segment_normal(&g_state->level, seg);
                vec4 seg_norm4 = H_SWIZZLE(seg_norm, 0, 1, 0, 0);
                seg_norm4[2] = 0.0;
                seg_norm4[3] = 0.0;
                if (dot(normalize(p_to_light), seg_norm4) > 0) continue;

                vec2 *verts[2];
                get_vertices(&g_state->level, seg, verts);

                vec2 seginter = intersect_seg(*verts[0], *verts[1], t->position, H_SWIZZLE(raypos, 0, 1));
                if (isnan(seginter[0])) continue;

                intersection = true;

                if (seg->portal) {
                    vec2 relinter = seginter - H_SWIZZLE(raypos, 0, 1);
                    vec2 p_to_light_2d =  H_SWIZZLE(wpos - raypos, 0, 1);
                    float t = dot(relinter, p_to_light_2d ) / dot( p_to_light_2d,  p_to_light_2d);

                    float d2d = magnitude(p_to_light_2d);
                    float iz = raypos[2] + t * ((wpos - raypos)[2]);

                    sector_t *other = h_array_get(&g_state->level.sections, seg->portal - 1);

                    float lower_window = fmaxf(current->floor, other->floor);
                    float upper_window = fminf(current->ceil, other->ceil);

                    if (iz > lower_window && iz < upper_window) {
                        current = other;
                        raypos = (vec4){seginter[0], seginter[1], iz, 0.0};
                        goto next_sector;
                    }
                }

                inter = seginter;

                current = nullptr;
                break;
            }

            if (!intersection) break;
            next_sector:;
        }

        if (!isnan(inter[0])) continue;

        f32 d = magnitude(p_to_light);
        f32 attenuation = 1.0f / (1.0f + d * d);

        f32 dot_factor = dot(norm, normalize(p_to_light));
        dot_factor = dot_factor<0?0:dot_factor;

        radiance = radiance + l->tint * l->intensity * attenuation * dot_factor;
    }
    radiance[0] = radiance[0]>1?1:radiance[0];
    radiance[1] = radiance[1]>1?1:radiance[1];
    radiance[2] = radiance[2]>1?1:radiance[2];
    return radiance;
}

ivec2 get_lightmap_size_from_surface(vec2 surface) {
    surface = surface / LIGHTMAP_SAMPLE_SPAN;
    return (ivec2){
        ceil(surface[0]),
        ceil(surface[1])
    };
}

lightmap_t generate_segment_lightmap(h_arena_t *allocator, struct segment_s *seg, struct sector_s *sector) {
    vec2 *verts[2];
    get_vertices(&g_state->level, seg, verts);

    f32 length = magnitude(*verts[1] - *verts[0]);
    f32 height = sector->ceil - sector->floor;

    ivec2 res = get_lightmap_size_from_surface((vec2){length, height});

    lightmap_t lightmap;

    lightmap.data = h_arena_alloc(allocator, res[0]*res[1] * sizeof(light_texel_t));
    lightmap.w = res[0];
    lightmap.h = res[1];

    // Light calculations
    for (i32 y = 0; y < lightmap.h; y++) {
        for (i32 x = 0; x < lightmap.w; x++) {
            f32 xratio = (f32)x / ((f32)lightmap.w - 1);
            f32 yratio = (f32)y / ((f32)lightmap.h - 1);
            vec2 wpos2 = *verts[1] + xratio * (*verts[0] - *verts[1]);
            vec4 wpos = H_SWIZZLE(wpos2, 0, 1, 0, 0);
            wpos[2] = sector->floor + yratio * (sector->ceil - sector->floor);
            wpos[3] = 0.0;
            vec4 norm = H_SWIZZLE(get_segment_normal(&g_state->level, seg), 0, 1, 0, 0);
            norm[2] = 0;
            norm[3] = 0;
            vec4 color = radiance_at_world_position(wpos, norm, sector);
            lightmap.data[x + y * lightmap.w] = vec4_to_hex(color);
        }
    }

    return lightmap;
}

lightmap_t generate_floor_lightmap(h_arena_t *allocator, struct sector_s *sector) {

    vec2 boundsize = H_SWIZZLE(sector->bounds, 2, 3);
    ivec2 res = get_lightmap_size_from_surface(boundsize);

    lightmap_t lightmap;
    lightmap.data = h_arena_alloc(allocator, res[0]*res[1] * sizeof(light_texel_t));
    lightmap.w = res[0];
    lightmap.h = res[1];

    vec2 boundpos = H_SWIZZLE(sector->bounds, 0, 1);

    // Light calculations
    for (i32 y = 0; y < lightmap.h; y++) {
        for (i32 x = 0; x < lightmap.w; x++) {
            vec2 ratio = {
                (f32)x / lightmap.w,
                (f32)y / lightmap.h
            };
            vec2 w_sector_relative_pos = ratio * boundsize;
            vec2 wpos2 = boundpos + w_sector_relative_pos;
            vec4 wpos = H_SWIZZLE(wpos2, 0, 1, 0, 0);
            wpos[2] = sector->floor;
            wpos[3] = 0.0;
            vec4 norm = {0,0,1,0};
            vec4 color = radiance_at_world_position(wpos, norm, sector);
            lightmap.data[x + y * lightmap.w] = vec4_to_hex(color);
        }
    }

    return lightmap;
}

lightmap_t generate_ceiling_lightmap(h_arena_t *allocator, struct sector_s *sector) {

    vec2 boundsize = H_SWIZZLE(sector->bounds, 2, 3);
    ivec2 res = get_lightmap_size_from_surface(boundsize);

    lightmap_t lightmap;
    lightmap.data = h_arena_alloc(allocator, res[0]*res[1] * sizeof(light_texel_t));
    lightmap.w = res[0];
    lightmap.h = res[1];

    vec2 boundpos = H_SWIZZLE(sector->bounds, 0, 1);

    // Light calculations
    for (i32 y = 0; y < lightmap.h; y++) {
        for (i32 x = 0; x < lightmap.w; x++) {
            vec2 ratio = {
                (f32)x / lightmap.w,
                (f32)y / lightmap.h
            };
            vec2 w_sector_relative_pos = ratio * boundsize;
            vec2 wpos2 = boundpos + w_sector_relative_pos;
            vec4 wpos = H_SWIZZLE(wpos2, 0, 1, 0, 0);
            wpos[2] = sector->ceil;
            wpos[3] = 0.0;
            vec4 norm = {0,0,-1,0};
            vec4 color = radiance_at_world_position(wpos, norm, sector);
            lightmap.data[x + y * res[0]] = vec4_to_hex(color);
        }
    }

    return lightmap;
}

level_lightmap_t *generate_level_lightmap(h_arena_t *allocator, struct level_s *level) {
    printf("Generating level lightmaps for %s...\n", level->level_name);

    level_lightmap_t *level_lightmap = h_arena_alloc(allocator, sizeof(level_lightmap_t));

    level_lightmap->allocator = h_arena_create("level_lightmap_arena");

    level_lightmap->light_data = h_arena_alloc(level_lightmap->allocator, level->segments.size * sizeof(lightmap_t));

    h_iter_t sector_iter = h_array_iter(&level->sections);
    int lightmap_idx = 0;
    H_FOREACH_PTR(sector_t, s, sector_iter) {
        printf("Generating sector lightmaps...\n");

        printf("Floor...\n");
        level_lightmap->light_data[lightmap_idx] = generate_floor_lightmap(level_lightmap->allocator, s);
        printf("Ceiling...\n");
        level_lightmap->light_data[lightmap_idx + 1] = generate_ceiling_lightmap(level_lightmap->allocator, s);

        printf("Segments...\n");
        for (int i=0;i<s->num_segments;++i) {
            segment_t *seg = s->first_segment + i;

            level_lightmap->light_data[lightmap_idx + 2 + i] = generate_segment_lightmap(level_lightmap->allocator, seg, s);
        }

        s->lightmaps = level_lightmap->light_data + lightmap_idx;

        lightmap_idx += s->num_segments + 2;
    }


    return level_lightmap;
}

void destroy_level_lightmap(level_lightmap_t *lightmap) {
    if (lightmap)
        h_arena_destroy(lightmap->allocator);
}
