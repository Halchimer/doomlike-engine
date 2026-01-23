#pragma once

#include <string.h>

#include "../../component_ids.h"
#include "entity.h"
#include "../error.h"

// Component implementations

typedef enum component_id_e {
#define X(Name,...) Name,
    ECS_COMPONENT_IDS(X)
#undef X
    MAX_COMPONENTS
} component_id_t;

static inline component_id_t get_component_id_from_name(const char *name) {
#define X(Name,...) if (strcmp(name, #Name) == 0) return Name;
    ECS_COMPONENT_IDS(X)
#undef X
    return -1;
}

static inline const char* get_component_name_from_id(component_id_t id) {
#define X(Name,...) if (Name == id) return #Name;
    ECS_COMPONENT_IDS(X)
#undef X
    return NULL;
}

static inline size_t get_component_size(component_id_t id) {
#define X(ID,Type) if (id == ID) return sizeof(Type);
    ECS_COMPONENT_IDS(X)
#undef X
    return 0;
}

#define X(ID, ...) extern void ID##_parse(void *in_component, h_array_t *words);
    ECS_COMPONENT_IDS(X)
#undef X

extern void (*g_component_parsers[MAX_COMPONENTS])(void *in_component, h_array_t *words);

// Component storage & management

#define COMPONENT_POOL_CAPACITY 512
typedef struct component_pool_s {
    size_t item_size;
    size_t size;
    void *data;
    entity_t owner_map[COMPONENT_POOL_CAPACITY];
} component_pool_t;

typedef RESULT(component_pool_t) component_pool_result_t;
component_pool_result_t init_component_pool(size_t item_size, h_arena_t *arena);

void *component_pool_alloc(component_pool_t *pool);
void component_pool_free(component_pool_t *pool, size_t idx);
void *component_pool_get(component_pool_t *pool, size_t idx);