#pragma once

/*
 *  When adding a component don't forget to add its ID to the macro bellow.
 */

#include "core/ecs/components/transform.h"
#include "core/ecs/components/sprite.h"
#include "core/ecs/components/billboard.h"
#include "core/ecs/components/camera.h"
#include "core/ecs/components/velocity.h"
#include "core/ecs/components/level_collider.h"
#include "core/ecs/components/sector_component.h"
#include "core/ecs/components/light.h"

// TODO: Create internal components
#define ECS_COMPONENT_IDS(X)    \
X(TRANSFORM , transform_t)      \
X(SPRITE    , sprite_t)         \
X(BILLBOARD , billboard_t)      \
X(CAMERA    , camera_t)         \
X(VELOCITY  , velocity_t)       \
X(LEVEL_COLLIDER, level_collider_t)     \
X(SECTOR_COMPONENT, sector_component_t) \
X(LIGHT , light_t)