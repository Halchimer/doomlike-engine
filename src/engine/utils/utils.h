#pragma once

#include <hcgmath.h>

#define PI 3.14159265358979323846
#define TO_RADIANS (PI/180.0f)
#define TO_DEGREES (180.0f/PI)
#define CLAMP(x,min,max) ((x)<(min)?(min):((x)>(max)?(max):(x)))
#define VISNAN(v) (v!=v)

f32 cross2(vec2 a, vec2 b);

// computes intersection of two line segments
vec2 intersect_seg(vec2 a0, vec2 a1, vec2 b0, vec2 b1);

struct intersect_circle_seg_result_s {
    bool intersect;
    vec2 a, b;
};
// returns NAN if no intersection, and the two intersection points in x,y and z,w respectively
struct intersect_circle_seg_result_s intersect_circle_seg(vec2 center, float radius, vec2 a0, vec2 a1);

vec2 rot2(vec2 v, float angle);

float frac(float v);

typedef struct clock_s {
    f64 last;
    f64 current;
} h_clock_t;

void clock_now(h_clock_t *clock);
f64 clock_delta(h_clock_t *clock);

vec4 hex_to_vec4(u32 hex);
u32 vec4_to_hex(vec4 color);

static inline int first_leading_zero(uint32_t x) {
    if (x == 0xFFFFFFFF) return -1;
    return __builtin_clz(~x);
}