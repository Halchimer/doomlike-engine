#include "transform.h"

#include "../component.h"

void TRANSFORM_parse(void *in_component, h_array_t *words) {
    transform_t *transform = in_component;

    char *end_ptr;
    transform->position[0] =    strtof(h_cstr(H_ARRAY_GET(h_string_t,*words, 3)), &end_ptr);
    transform->position[1] =    strtof(h_cstr(H_ARRAY_GET(h_string_t,*words, 4)), &end_ptr);
    transform->scale[0] =       strtof(h_cstr(H_ARRAY_GET(h_string_t,*words, 5)), &end_ptr);
    transform->scale[1] =       strtof(h_cstr(H_ARRAY_GET(h_string_t,*words, 6)), &end_ptr);
    transform->rotation =       strtof(h_cstr(H_ARRAY_GET(h_string_t,*words, 7)), &end_ptr);
}