#include "window.h"

window_result_t init_window(const char *title, i32 w, i32 h) {
    window_t window = {0};
    window.sdl_window = SDL_CreateWindow(title, w, h, 0);
    if (!window.sdl_window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return (window_result_t){.isA=false,.b=SDL_APP_FAILURE};
    }
    return (window_result_t){.isA=true,.a=window};
}

void destroy_window(window_t *window) {
    SDL_DestroyWindow(window->sdl_window);
}
