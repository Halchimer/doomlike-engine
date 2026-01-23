#include "sector_component.h"

#include "../component.h"

void SECTOR_COMPONENT_parse(void *in_component, h_array_t *words) {
    sector_component_t *sector_component = in_component;

    char *end_ptr;
    sector_component->sector = strtol(h_cstr(H_ARRAY_GET(h_string_t,*words, 3)), &end_ptr, 10);
}