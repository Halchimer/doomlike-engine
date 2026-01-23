#include "ecs.h"

#include "../../utils/utils.h"

bool compare_signatures_ptr(void *a, void *b) {
    return *(signature_t *)a == *(signature_t *)b;
}

// Structs and functions for archetype hashmap

typedef struct archetype_pair_s {
    signature_t signature;
    archetype_t archetype;
} archetype_pair_t;

u32 hash_archetype_pair(void *pair) {
    archetype_pair_t *ap = (archetype_pair_t *)pair;

    return h_hash(h_pcg_hash, &ap->signature, sizeof(signature_t));
}

bool compare_archetype_pairs(void *a, void *b) {
    archetype_pair_t *ap = (archetype_pair_t *)a;
    archetype_pair_t *bp = (archetype_pair_t *)b;
    return ap->signature == bp->signature;
}

#define GET_ARCHETYPE_FROM_HASHMAP(World, Signature) ({archetype_pair_t pair = {.signature = Signature}; &((archetype_pair_t*)h_hashmap_get(&World->archetype_map, &pair))->archetype;})

//

world_t init_ecs_world(h_arena_t *arena) {
    world_t world = {0};
    world.archetype_map = H_CREATE_HASHMAP(archetype_pair_t, 128, hash_archetype_pair, compare_archetype_pairs);
    for (component_id_t id = 0; id < MAX_COMPONENTS; ++id) {
        component_pool_result_t res = init_component_pool(get_component_size(id), arena);
        if (!res.isA) continue;
        world.component_pools[id] = res.a;
    }
    printf("Initialized ECS world.\n");
    return world;
}

void destroy_ecs_world(world_t *world) {
    h_hashmap_free(&world->archetype_map);
    printf("Destroyed ECS world.\n");
}

void add_entity_to_archetype(world_t *world, entity_t eid) {
    entity_record_t *entity = &world->entity_manager.records[eid];
    archetype_pair_t dummy = {.signature = entity->signature};
    archetype_pair_t *archpair = h_hashmap_get(&world->archetype_map, &dummy);
    archetype_t *archetype;
    if (!archpair) {
        archetype_t narchetype = {0};
        narchetype.signature = entity->signature;
        archetype_pair_t pair = {.signature = entity->signature, .archetype = narchetype};
        archetype = &((archetype_pair_t*)h_hashmap_put(&world->archetype_map, &pair))->archetype;
    }
    else archetype = &archpair->archetype;

    entity->archetype_index = archetype->num_entities++;
    archetype->entities[entity->archetype_index] = eid;
}

void remove_entity_from_archetype(world_t *world, entity_t eid) {
    entity_record_t *entity = &world->entity_manager.records[eid];
    archetype_t *archetype = GET_ARCHETYPE_FROM_HASHMAP(world, entity->signature);
    archetype->num_entities--;
    archetype->entities[world->entity_manager.records[eid].archetype_index] = archetype->entities[archetype->num_entities];
    world->entity_manager.records[archetype->entities[archetype->num_entities]].archetype_index = world->entity_manager.records[eid].archetype_index;
}

entity_t create_entity(world_t *world) {
    if (world->entity_manager.free_mask == ((_BitInt(MAX_ENTITIES))1<<(MAX_ENTITIES - 1))) return -1;
    entity_t eid =  __builtin_ctz(~world->entity_manager.free_mask);

    entity_record_t *entity = &world->entity_manager.records[eid];
    entity->signature = 0;
    entity->component_map[0] = 0;
    world->entity_manager.free_mask |= (1 << eid);

    add_entity_to_archetype(world, eid);

    printf("Created empty entity %lld\n", eid );

    return eid;
}

bool exists(world_t *world, entity_t eid) {
    if (eid < 0 || eid >= MAX_ENTITIES) return false;
    return (world->entity_manager.free_mask & (1 << eid));
}

bool has_component(world_t *world, entity_t eid, component_id_t id) {
    if (!exists(world, eid)) return false;
    return (world->entity_manager.records[eid].signature & (1 << id));
}

void *add_component(world_t *world, entity_t eid, component_id_t id) {
    if (!exists(world, eid)) return nullptr;
    if (has_component(world, eid, id)) {
        return component_pool_get(&world->component_pools[id], world->entity_manager.records[eid].component_map[id]);
    }

    remove_entity_from_archetype(world, eid);

    void *component = component_pool_alloc(&world->component_pools[id]);
    world->component_pools[id].owner_map[world->component_pools[id].size-1] = eid;
    world->entity_manager.records[eid].component_map[id] = world->component_pools[id].size-1;
    world->entity_manager.records[eid].signature |= (1 << id);

    add_entity_to_archetype(world, eid);

    printf("Added component %s to entity %lld\n", get_component_name_from_id(id), eid);

    return component;
}

void remove_component(world_t *world, entity_t eid, component_id_t id) {
    if (!exists(world, eid)) return;
    if (!has_component(world, eid, id)) return;

    remove_entity_from_archetype(world, eid);

    size_t cidx = world->entity_manager.records[eid].component_map[id];

    component_pool_free(&world->component_pools[id], cidx);
    world->entity_manager.records[eid].signature &= ~(1 << id);

    entity_t owner = world->component_pools[id].owner_map[world->component_pools[id].size-1];
    world->component_pools[id].owner_map[cidx] = owner;
    world->entity_manager.records[owner].component_map[id] = cidx;

    printf("Removed component %s to entity %lld\n", get_component_name_from_id(id), eid);

    add_entity_to_archetype(world, owner);
}

void * get_component(world_t *world, entity_t eid, component_id_t id) {
    if (!exists(world, eid)) return nullptr;
    if (!has_component(world, eid, id)) return nullptr;
    return component_pool_get(&world->component_pools[id], world->entity_manager.records[eid].component_map[id]);
}

void destroy_entity(world_t *world, entity_t eid) {
    if (!exists(world, eid)) return;
    remove_entity_from_archetype(world, eid);
    for (component_id_t id = 0; id < MAX_COMPONENTS; ++id) {
        size_t cidx = world->entity_manager.records[eid].component_map[id];

        component_pool_free(&world->component_pools[id], cidx);
        world->entity_manager.records[eid].signature &= ~(1 << id);

        entity_t owner = world->component_pools[id].owner_map[world->component_pools[id].size-1];
        world->component_pools[id].owner_map[cidx] = owner;
        world->entity_manager.records[owner].component_map[id] = cidx;
    }
    world->entity_manager.free_mask &= ~(1 << eid);

    printf("Destroyed entity %lld", eid);
}

// Querying

entity_query_t query_entities(world_t *world, size_t n, component_id_t ids[]) {
    signature_t signature = 0;
    for (size_t i = 0; i < n; ++i) signature |= (1 << ids[i]);

    size_t entity_arr_cap = 16;
    entity_t *entities = calloc(16, sizeof(entity_t));
    size_t num_entities = 0;

    archetype_pair_t *pairs = world->archetype_map.kvpool;
    archetype_pair_t *pair = pairs;

    while (pair < pairs + world->archetype_map.size) {
        archetype_t *arch = &pair->archetype;

        if (!((arch->signature & signature) == signature)) goto CONTINUE;

        if (num_entities + arch->num_entities > entity_arr_cap) {
            entity_arr_cap *= 2;
            entity_arr_cap = entity_arr_cap<(num_entities+arch->num_entities) ? (num_entities+arch->num_entities) : entity_arr_cap;
            entities = realloc(entities, entity_arr_cap * sizeof(entity_t));
        }
        memcpy(entities + num_entities, arch->entities, arch->num_entities * sizeof(entity_t));
        num_entities += arch->num_entities;

        CONTINUE :
        pair++;
    }
    return (entity_query_t) {.entities = entities, .num_entities = num_entities};
}

entity_query_t query_entities_explicit(world_t *world, size_t n, component_id_t ids[]) {
    signature_t signature = 0;
    for (size_t i = 0; i < n; ++i) signature |= (1 << ids[i]);
    archetype_t *archetype = GET_ARCHETYPE_FROM_HASHMAP(world, signature);
    return (entity_query_t) {.entities = archetype->entities, .num_entities = archetype->num_entities};
}

void *entity_query_next(h_iter_t *iter) {
    entity_query_t *q = (entity_query_t*)iter->collection;
    void *val = iter->state;
    if (iter->hasnext(iter))
        iter->state+=sizeof(entity_t);
    return val;
}
bool entity_query_hasnext(h_iter_t *iter) {
    entity_query_t *q = (entity_query_t*)iter->collection;
    return (const entity_t *)iter->state < q->entities+q->num_entities;
}

h_iter_t query_entities_iter(entity_query_t *query) {
    return (h_iter_t){
        .collection = query,
        .state = query->entities,
        .next = &entity_query_next,
        .hasnext = &entity_query_hasnext
    };
}

void *component_pool_next(h_iter_t *iter) {
    component_pool_t *cpool = (component_pool_t*)iter->collection;
    void *val = iter->state;
    if (iter->hasnext(iter))
        iter->state+=cpool->item_size;
    return val;
}
bool component_pool_hasnext(h_iter_t *iter) {
    component_pool_t *cpool = (component_pool_t*)iter->collection;
    return iter->state < cpool->data+cpool->size*cpool->item_size;
}

h_iter_t component_pool_iter(world_t *world, component_id_t id) {
    world->component_pools[id];
    return (h_iter_t){
        &world->component_pools[id],
        world->component_pools[id].data,
        &component_pool_next,
        &component_pool_hasnext};
}

void * get_singleton_component(world_t *world, component_id_t id) {
    return component_pool_get(&world->component_pools[id], 0);
}
