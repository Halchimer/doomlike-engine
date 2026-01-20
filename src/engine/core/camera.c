#include "camera.h"

#include "../utils/utils.h"

camera_t *g_camera;

camera_t init_camera(vec2 pos, f32 rot, f32 fov, f32 near) {
    return (camera_t){pos,rot,fov,near, tan(fov) * near * 2, 0};
}
vec2 camera_forward(camera_t *camera) {
    return rot2((vec2){0.0, 1.0}, camera->rotation);
}

vec2 camera_raydir(camera_t *camera, float rx) {
    if (!camera) return (vec2){0,0};
    float dm = 2.0f * (rx - 0.5f);
    dm *= tanf(camera->fov) * camera->near;
    vec2 w = (vec2){dm, 1.0f};
    return normalize(rot2(w, camera->rotation));
}
