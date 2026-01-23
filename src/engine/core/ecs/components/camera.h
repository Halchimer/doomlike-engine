#pragma once

#include <hcgmath.h>

#include "sector_component.h"
#include "transform.h"

typedef struct camera_s {
    transform_t *transform;
    sector_component_t *sector;
    f32 fov;
    f32 near;
    float near_plane_world_width;
} camera_t;
extern camera_t *g_camera;

camera_t init_camera(transform_t *transform, f32 fov, f32 near);
vec2 camera_forward(camera_t *camera);
vec2 camera_raydir(camera_t *camera, float rx);

void set_active_camera(camera_t *camera);