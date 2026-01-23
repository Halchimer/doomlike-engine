#include "level_collider.h"

#include "../component.h"

void LEVEL_COLLIDER_parse(void *in_component, h_array_t *words) {
    level_collider_t *collider = in_component;

    char* end_ptr;
    collider->radius = strtof(h_cstr(H_ARRAY_GET(h_string_t,*words, 3)), &end_ptr);
}