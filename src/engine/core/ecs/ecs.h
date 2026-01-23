#pragma once

#include "component.h"

#define MAX_ENTITIES 1024

typedef _BitInt(MAX_COMPONENTS) signature_t;

typedef struct entity_record_s {
    signature_t signature;
    size_t archetype_index;
    size_t component_map[MAX_COMPONENTS];
} entity_record_t;

struct entity_manager_s {
    entity_record_t records[MAX_ENTITIES];
    unsigned _BitInt(MAX_ENTITIES) free_mask;
};

typedef struct archetype_s {
    entity_t entities[MAX_ENTITIES];
    size_t num_entities;
    signature_t signature;
} archetype_t;

typedef struct world_s {
    h_hashmap_t archetype_map; // k : signature v : archetype
    struct entity_manager_s entity_manager;
    component_pool_t component_pools[MAX_COMPONENTS];
} world_t;

world_t init_ecs_world(h_arena_t *arena);
void destroy_ecs_world(world_t *world);
entity_t create_entity(world_t *world);
bool exists(world_t *world, entity_t eid);
bool has_component(world_t *world, entity_t eid, component_id_t id);
void *add_component(world_t *world, entity_t eid, component_id_t id);
void remove_component(world_t *world, entity_t eid, component_id_t id);
void *get_component(world_t *world, entity_t eid, component_id_t id);
void destroy_entity(world_t *world, entity_t eid);

// Querying

typedef struct entity_query_s {
    const entity_t *entities;
    size_t num_entities;
} entity_query_t ;

entity_query_t query_entities(world_t *world, size_t n, component_id_t ids[]);
entity_query_t query_entities_explicit(world_t *world, size_t n, component_id_t ids[]);

h_iter_t query_entities_iter(entity_query_t *query);
h_iter_t component_pool_iter(world_t *world, component_id_t id);

void *get_singleton_component(world_t *world, component_id_t id);