#pragma once

#include <hclib.h>

#define ASSET_TYPES(X) \
    X(ASSET_TEXTURE)   \
    X(ASSET_AUDIO)     \

// Generates asset types enum
typedef enum asset_type_e {
#define X(Name) Name,
    ASSET_TYPES(X)
#undef X
    MAX_ASSET_TYPE
} asset_type_t;

// Asset loading functions
#define X(Name) extern void Name##_load(void *out, char *src);
ASSET_TYPES(X)
#undef X

typedef struct asset_s {
    asset_type_t type;
    void *data;
    i32 refcount;
} asset_t;

typedef struct asset_manager_s {
    h_hashmap_t assets;
} asset_manager_t;
asset_manager_t *g_asset_manager;

typedef struct asset_hande_t {
    i32 idx;
};


#undef ASSET_TYPES