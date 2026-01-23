#include "component.h"

#include <hclib.h>

#include "../app.h"

void (*g_component_parsers[MAX_COMPONENTS])(void *in_component, h_array_t *words) = {
#define X(ID, ...) &ID##_parse,
    ECS_COMPONENT_IDS(X)
#undef X
};

component_pool_result_t init_component_pool(size_t item_size, h_arena_t *arena) {
    if (!g_state) return (component_pool_result_t){.isA = false, .b = 0};
    component_pool_t pool = {0};
    pool.item_size = item_size;
    pool.data = h_arena_alloc(arena, item_size * COMPONENT_POOL_CAPACITY);
    return (component_pool_result_t){.isA = true, .a = pool};
}

void * component_pool_alloc(component_pool_t *pool) {
    return pool->data + pool->item_size * (pool->size++);
}

void component_pool_free(component_pool_t *pool, size_t idx) {
    if (idx >= pool->size) return;
    pool->size--;
    memmove(
        pool->data + idx * pool->item_size,
        pool->data + pool->size * pool->item_size, pool->item_size
        );
    pool->owner_map[idx] = pool->owner_map[pool->size];
}

void * component_pool_get(component_pool_t *pool, size_t idx) {
    if (idx < 0 || idx >= pool->size) return NULL;
    return pool->data + idx * pool->item_size;
}
