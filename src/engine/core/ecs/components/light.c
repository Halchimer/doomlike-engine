#include "light.h"

#include "../component.h"

void LIGHT_parse(void *in_component, h_array_t *words) {
    light_t *light = (light_t *)in_component;

    char *end_ptr;
    light->tint[0] = strtof(h_cstr(H_ARRAY_GET(h_string_t,*words, 3)), &end_ptr);
    light->tint[1] = strtof(h_cstr(H_ARRAY_GET(h_string_t,*words, 4)), &end_ptr);
    light->tint[2] = strtof(h_cstr(H_ARRAY_GET(h_string_t,*words, 5)), &end_ptr);
    light->tint[3] = 1.0;

    light->intensity = strtof(h_cstr(H_ARRAY_GET(h_string_t,*words, 6)), &end_ptr);

    light->height = strtof(h_cstr(H_ARRAY_GET(h_string_t,*words, 7)), &end_ptr);
}