#include "velocity.h"

#include "../component.h"

void VELOCITY_parse(void *in_component, h_array_t *words) {
    velocity_t *velocity = in_component;
    char *end_ptr;
    velocity->v[0] = strtof(h_cstr(H_ARRAY_GET(h_string_t,*words, 3)), &end_ptr);
    velocity->v[1] = strtof(h_cstr(H_ARRAY_GET(h_string_t,*words, 4)), &end_ptr);
}