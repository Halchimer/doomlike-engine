#include "utils.h"

#include <time.h>

#include "SDL3/SDL_timer.h"

f32 cross2(vec2 a, vec2 b) {
    b = H_SWIZZLE(b, 1,0);
    vec2 c = a*b;
    return c[0] - c[1];
}

vec2 intersect_seg(vec2 a0, vec2 a1, vec2 b0, vec2 b1) {
    vec2 r = a1 - a0;
    vec2 s = b1 - b0;
    float t = cross2(b0 - a0, s / cross2(r, s));
    float u = cross2(b0 - a0, r / cross2(r, s));

    if (cross2(r, s) != 0 && t >= 0 && t <= 1 && u >= 0 && u <= 1)
        return a0 + t * r;
    return (vec2){NAN, NAN};
}

struct intersect_circle_seg_result_s intersect_circle_seg(vec2 center, float radius, vec2 a0, vec2 a1) {
    vec2 dir = a1 - a0;
    vec2 ctoseg = a0 - center;

    float a = dot(dir, dir);
    float b = 2 * dot(dir, ctoseg);
    float c = dot(ctoseg, ctoseg) - radius*radius;

    float discr = b*b - 4*a*c;
    if (discr < 0) return (struct intersect_circle_seg_result_s){false};

    float t1 = (-b - sqrtf(discr))/(2*a);
    float t2 = (-b + sqrtf(discr))/(2*a);

    vec2 p0 = {NAN, NAN};
    if (t1 >= 0 && t1 <= 1) p0 = a0 + t1 * dir;

    vec2 p1 = {NAN, NAN};
    if (t2 >= 0 && t2 <= 1) p1 = a0 + t2 * dir;

    if (isnan(p0[0])) p0 = p1;
    if (isnan(p1[0])) p1 = p0;

    return (struct intersect_circle_seg_result_s){true, p0, p1};
}

vec2 rot2(vec2 v, float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    return (vec2){v[0]*c - v[1]*s, v[0]*s + v[1]*c};
}

float frac(float v) {
    return v - floorf(v);
}

void clock_now(h_clock_t *clock) {
    clock->last = clock->current;
    clock->current = (f64)SDL_GetTicks();
}

f64 clock_delta(h_clock_t *clock) {
    return clock->current - clock->last;
}

vec4 hex_to_vec4(u32 hex) {
    return (vec4){
        (f32)((hex & 0xFF000000)>>24)/255,
        (f32)((hex & 0x00FF0000)>>16)/255,
        (f32)((hex & 0x0000FF00)>>8)/255,
        (f32)(hex & 0x000000FF)/255
    };
}
u32 vec4_to_hex(vec4 color) {
    return ((u32)(color[0] * 255) << 24) | ((u32)(color[1] * 255) << 16) | ((u32)(color[2] * 255) << 8) | ((u32)(color[3] * 255));
}
