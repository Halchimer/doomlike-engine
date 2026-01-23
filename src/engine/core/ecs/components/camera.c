#include "camera.h"

#include "../component.h"
#include "../../../utils/utils.h"

camera_t *g_camera;

void CAMERA_parse(void *in_component, h_array_t *words) {
    // pour l'instant on peut ignorer, je créé le joueur avec sa cam manuellement
}

camera_t init_camera(transform_t *transform, f32 fov, f32 near) {
    return (camera_t){transform ,nullptr , fov , near, tan(fov) * near * 2};
}
vec2 camera_forward(camera_t *camera) {
    return rot2((vec2){0.0, 1.0}, camera->transform->rotation);
}

[[deprecated]]
vec2 camera_raydir(camera_t *camera, float rx) {
    if (!camera) return (vec2){0,0};
    float dm = 2.0f * (rx - 0.5f);
    dm *= tanf(camera->fov) * camera->near;
    vec2 w = (vec2){dm, 1.0f};
    return normalize(rot2(w, camera->transform->rotation));
}

void set_active_camera(camera_t *camera) {
    g_camera = camera;
}
