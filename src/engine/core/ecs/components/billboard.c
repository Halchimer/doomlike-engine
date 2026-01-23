#include "billboard.h"

#include "../component.h"

void BILLBOARD_parse(void *in_component, h_array_t *words) {
    billboard_t *billboard = in_component;

    char* end_ptr;
    billboard->section = strtol(h_cstr(H_ARRAY_GET(h_string_t,*words, 3)), &end_ptr, 10);
    billboard->z = strtof(h_cstr(H_ARRAY_GET(h_string_t,*words, 4)), &end_ptr);
    (void*){};
}