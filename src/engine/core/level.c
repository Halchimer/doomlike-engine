#include "level.h"

#include "texture_atlas.h"
#include "../utils/utils.h"

void get_vertices(level_t *level, segment_t *segment, vec2 *verts[2]) {
        verts[0] = h_array_get(&level->vertices, segment->vertices[0]);
        verts[1] = h_array_get(&level->vertices, segment->vertices[1]);
}

vec2 get_segment_normal(level_t *level, segment_t *segment) {
    vec2 *verts[2];
    get_vertices(level, segment, verts);
    vec2 v = *verts[0];
    vec2 w = *verts[1];
    vec2 a = w - v;
    vec2 norm = (vec2){-a[1], a[0]};
    return normalize(norm);
}

vec2 get_section_center(level_t *level, sector_t *section) {
    segment_t *first = section->first_segment;
    vec2 center = {0};
    for (int i = 0;i<section->num_segments;++i) {
        vec2 *verts[2];
        get_vertices(level, first, verts);
        vec2 segcenter = *verts[0] + (*verts[1] - *verts[0])/2;
        center = center + segcenter;
        first++;
    }
    center = center / (float)section->num_segments;
    return center;
}

/**
 * Loads a level from the specified file path and parses its contents into a level structure.
 *
 * The level file is expected to contain vertex, wall, and section data which are represented
 * by specific line tokens:
 * - "v": Indicates a vertex definition with x and y coordinates.
 * - "w": Indicates a wall or segment definition with vertices, portal, texture, and UV details.
 * - "s": Indicates a section definition with segments, floor/ceiling heights, and texture/UV details.
 *
 * Comments (lines beginning with '#') and empty lines are ignored during processing.
 *
 * @param path The file path to the level file to be loaded. Should be a valid readable file path.
 * @return A populated level_t structure representing the loaded level, including vertices,
 *         segments, and sections.
 */
level_t load_level(char const *path) {

    printf("Loading level %s\n", path);
    h_clock_t clock = {0};
    clock_now(&clock);

    // Open file & gather data

    FILE *f = fopen(path, "r");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *data = malloc(fsize + 1);
    size_t read_bytes = fread(data, 1, fsize, f);
    if (read_bytes != fsize) {
        fprintf(stderr, "Warning : read %zu bytes from level file, expected %ld\n", read_bytes, fsize);
    }
    fclose(f);
    data[read_bytes] = 0;

    h_string_t sdata = h_tostring(data);

    h_arena_t *level_arena = h_arena_create("LevelArena");

    // ECS Parsing

    world_t world = init_ecs_world(level_arena);

    // Parsing

    h_array_t vertices = H_CREATE_ARRAY(vec2, 1024);
    h_array_t segments = H_CREATE_ARRAY(segment_t, 512);
    h_array_t sector = H_CREATE_ARRAY(sector_t, 256);

    h_array_t lines = h_split_string(sdata, '\n');
    h_iter_t lineiter = h_array_iter(&lines);

    // prefixes
    h_string_t vs = h_tostring("v");
    h_string_t ws = h_tostring("w");
    h_string_t s = h_tostring("s");

    h_string_t ep = h_tostring("e");
    h_string_t cp = h_tostring("c");

    H_FOREACH_PTR(h_string_t, line, lineiter) {
        if (line->size == 0) goto free_continue;
        if (h_cstr(*line)[0] == '#') goto free_continue;

        h_array_t token = h_split_string(*line, ' ');

        if (h_string_eq_ptr(h_array_get(&token, 0), &vs )) // vertex
        {
            char *end_ptr;
            float x = strtof(h_cstr(H_ARRAY_GET(h_string_t,token, 1)), &end_ptr);
            float y = strtof(h_cstr(H_ARRAY_GET(h_string_t,token, 2)), &end_ptr);
            vec2 v = (vec2){x, y};
            H_ARRAY_PUSH(vec2, vertices, v);
        }
        else if (h_string_eq_ptr(h_array_get(&token, 0), &ws)) // wall
        {
            char *end_ptr;
            i32 portal = strtol(h_cstr(H_ARRAY_GET(h_string_t,token, 1)), &end_ptr, 10);
            i32 v1 = strtol(h_cstr(H_ARRAY_GET(h_string_t,token, 2)), &end_ptr, 10);
            i32 v2 = strtol(h_cstr(H_ARRAY_GET(h_string_t,token, 3)), &end_ptr, 10);
            i32 texid = get_atlas_index_from_name(h_cstr(H_ARRAY_GET(h_string_t,token, 4)));
            f32 uvs[4] = {
                strtof(h_cstr(H_ARRAY_GET(h_string_t,token, 5)), &end_ptr),
                strtof(h_cstr(H_ARRAY_GET(h_string_t,token, 6)), &end_ptr),
                strtof(h_cstr(H_ARRAY_GET(h_string_t,token, 7)), &end_ptr),
                strtof(h_cstr(H_ARRAY_GET(h_string_t,token, 8)), &end_ptr),
            };
            segment_t s = (segment_t){v1, v2, portal, texid,
                uvs[0],
                uvs[1],
                uvs[2],
                uvs[3]
            };
            H_ARRAY_PUSH(segment_t, segments, s);
        }
        else if (h_string_eq_ptr(h_array_get(&token, 0), &s)) // section
        {
            char *end_ptr;
            int first = strtol(h_cstr(H_ARRAY_GET(h_string_t,token, 1)), &end_ptr, 10);
            size_t n = strtol(h_cstr(H_ARRAY_GET(h_string_t,token, 2)), &end_ptr, 10);
            float floor = strtof(h_cstr(H_ARRAY_GET(h_string_t,token, 3)), &end_ptr);
            float ceil = strtof(h_cstr(H_ARRAY_GET(h_string_t,token, 4)), &end_ptr);
            i32 floor_texid = get_atlas_index_from_name(h_cstr(H_ARRAY_GET(h_string_t,token, 5)));
            i32 ceil_texid = get_atlas_index_from_name(h_cstr(H_ARRAY_GET(h_string_t,token, 6)));
            f32 floor_uv0 = strtof(h_cstr(H_ARRAY_GET(h_string_t,token, 7)), &end_ptr);
            f32 floor_uv1 = strtof(h_cstr(H_ARRAY_GET(h_string_t,token, 8)), &end_ptr);
            f32 ceil_uv0 = strtof(h_cstr(H_ARRAY_GET(h_string_t,token, 9)), &end_ptr);
            f32 ceil_uv1 = strtof(h_cstr(H_ARRAY_GET(h_string_t,token, 10)), &end_ptr);

            sector_t sec = (sector_t){h_array_get(&segments, first), n, floor, ceil, floor_texid, ceil_texid, floor_uv0, floor_uv1, ceil_uv0, ceil_uv1};
            H_ARRAY_PUSH(sector_t, sector, sec);
        }
        else if (h_string_eq_ptr(h_array_get(&token, 0), &ep)) // entity
        {
            entity_t e = create_entity(&world);
            if (e < 0) {
                fprintf(stderr, "Failed to create entity\n");
                goto free_continue;
            }

            // Whatever is after the 'e' prefix is just comments for now
            // TODO : implement an entity archetype / preset system
        }
        else if (h_string_eq_ptr(h_array_get(&token, 0), &cp)) // component
        {
            char* end_ptr;
            entity_t e = strtol(h_cstr(H_ARRAY_GET(h_string_t,token, 1)), &end_ptr, 10);
            component_id_t id = get_component_id_from_name(h_cstr(H_ARRAY_GET(h_string_t,token, 2)));
            void* comp = add_component(&world, e, id);
            if (!comp) {
                fprintf(stderr, "Failed to add component %s to entity %d\n", h_cstr(H_ARRAY_GET(h_string_t,token, 2)), e);
                goto free_continue;
            }
            g_component_parsers[id](comp, &token);
        }
        else {
            fprintf(stderr, "Unknown token %s\n", h_cstr(H_ARRAY_GET(h_string_t,token, 0)));
        }

        free_continue : {}
        h_iter_t tokiter = h_array_iter(&token);
        //H_FOREACH_PTR(h_string_t, tok, tokiter) free(tok->cstr);
        //if (token.cap > 0)
        //    h_array_free(&token);

        //free(h_cstr(*line));
        ++line;
    }
    h_array_free(&lines);

    clock_now(&clock);
    printf("Loaded level in %f ms\n", clock_delta(&clock));

    return (level_t){.level_name = path,.vertices = vertices, .segments = segments,.sections = sector, .world = world, .level_arena = level_arena};
}

void destroy_level(level_t *level) {
    h_array_free(&level->vertices);
    h_array_free(&level->segments);
    h_array_free(&level->sections);
    destroy_ecs_world(&level->world);
    h_arena_destroy(level->level_arena);
}
