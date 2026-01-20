#pragma once

#include <SDL3/SDL.h>
#include <hclib.h>

#include "error.h"

typedef struct window_s {
    SDL_Window *sdl_window;
} window_t;
static window_t *g_window;

typedef RESULT(window_t) window_result_t;

window_result_t init_window(const char *title, i32 w, i32 h);
void destroy_window(window_t *window);