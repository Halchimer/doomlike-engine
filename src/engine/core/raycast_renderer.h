#pragma once

/*

    provides rendering function for game and 3d editor layers.
    TODO : Implement billboards rendering

 */

#define EYE_Z 1.65F

struct renderer_s;

void raycaster_render(void *state, struct renderer_s *renderer);