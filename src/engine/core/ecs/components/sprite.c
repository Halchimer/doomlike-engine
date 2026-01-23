#include "sprite.h"

#include "../component.h"

void SPRITE_parse(void *in_component, h_array_t *words) {
    sprite_t *sprite = in_component;

    sprite->atlas_index = get_atlas_index_from_name(h_cstr(H_ARRAY_GET(h_string_t,*words, 3)));
}