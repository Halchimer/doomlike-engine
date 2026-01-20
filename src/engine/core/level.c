#include "level.h"

#include "texture_atlas.h"
#include "../utils/utils.h"

vec2 get_segment_normal(segment_t *segment) {
    vec2 v = *segment->vertices[0];
    vec2 w = *segment->vertices[1];
    vec2 a = w - v;
    vec2 norm = (vec2){-a[1], a[0]};
    return normalize(norm);
}

vec2 get_section_center(section_t *section) {
    segment_t *first = section->first_segment;
    vec2 center = {0};
    for (int i = 0;i<section->num_segments;++i) {
        vec2 segcenter = *first->vertices[0] + (*first->vertices[1] - *first->vertices[0])/2;
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
    fread(data, 1, fsize, f);
    fclose(f);
    data[fsize] = 0;

    h_string_t sdata = h_tostring(data);

    // Parsing

    h_array_t vertices = H_CREATE_ARRAY(vec2, 1024);
    h_array_t segments = H_CREATE_ARRAY(segment_t, 512);
    h_array_t sector = H_CREATE_ARRAY(section_t, 256);

    h_array_t lines = h_split_string(sdata, '\n');
    h_iter_t lineiter = h_array_iter(&lines);
    H_FOREACH_PTR(h_string_t, line, lineiter) {
        if (line->size == 0) goto free_continue;
        if (h_cstr(*line)[0] == '#') goto free_continue;

        h_array_t token = h_split_string(*line, ' ');

        h_string_t vs = h_tostring("v");
        h_string_t ws = h_tostring("w");
        h_string_t s = h_tostring("s");

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
                strtol(h_cstr(H_ARRAY_GET(h_string_t,token, 5)), &end_ptr, 10),
                strtol(h_cstr(H_ARRAY_GET(h_string_t,token, 6)), &end_ptr, 10),
                strtol(h_cstr(H_ARRAY_GET(h_string_t,token, 7)), &end_ptr, 10),
                strtol(h_cstr(H_ARRAY_GET(h_string_t,token, 8)), &end_ptr, 10),
            };
            segment_t s = (segment_t){h_array_get(&vertices, v1), h_array_get(&vertices, v2), portal, texid,
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

            section_t sec = (section_t){h_array_get(&segments, first), n, floor, ceil, floor_texid, ceil_texid, floor_uv0, floor_uv1, ceil_uv0, ceil_uv1};
            H_ARRAY_PUSH(section_t, sector, sec);
        }
        else {
            fprintf(stderr, "Unknown token %s\n", h_cstr(*line));
        }

        h_iter_t tokiter = h_array_iter(&token);
        H_FOREACH_PTR(h_string_t, tok, tokiter) free(tok->cstr);
        h_array_free(&token);

        free_continue : {}
        free(h_cstr(*line));
        ++line;
    }
    h_array_free(&lines);

    clock_now(&clock);
    printf("Loaded level in %f ms\n", clock_delta(&clock));

    return (level_t){.level_name = path,.vertices = vertices, .segments = segments,.sections = sector};
}

void destroy_level(level_t *level) {
    h_array_free(&level->vertices);
    h_array_free(&level->segments);
    h_array_free(&level->sections);
}
