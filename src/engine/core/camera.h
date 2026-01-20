#pragma once

#include <hcgmath.h>

typedef struct camera_s {
    vec2 position;
    f32 rotation;
    f32 fov;
    f32 near;
    float near_plane_world_width;
    i32 sector;
} camera_t;
extern camera_t *g_camera;

camera_t init_camera(vec2 pos, f32 rot, f32 fov, f32 near);
vec2 camera_forward(camera_t *camera);
vec2 camera_raydir(camera_t *camera, float rx);