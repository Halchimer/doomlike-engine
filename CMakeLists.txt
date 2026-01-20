cmake_minimum_required(VERSION 4.1)
project(doomlike C)

set(CMAKE_C_STANDARD 23)

add_executable(doomlike src/main.c
        src/engine/implementations/hclib.c
        src/engine/core/app.c
        src/engine/core/app.h
        src/engine/core/renderer.c
        src/engine/core/renderer.h
        src/engine/core/window.c
        src/engine/core/window.h
        src/engine/utils/version.h
        src/engine/core/error.h
        src/engine/utils/utils.c
        src/engine/utils/utils.h
        src/engine/core/camera.c
        src/engine/core/camera.h
        src/engine/core/level.c
        src/engine/core/level.h
        src/engine/core/layer.c
        src/engine/core/layer.h
        src/engine/layers/game_layer.c
        src/engine/layers/game_layer.h
        src/engine/core/raycast_renderer.c
        src/engine/core/raycast_renderer.h
        src/engine/layers/level_editor_layer.c
        src/engine/layers/level_editor_layer.h
        src/engine/implementations/nuklear.c
        src/engine/core/texture.c
        src/engine/core/texture.h
        src/engine/core/texture_atlas.c
        src/engine/core/texture_atlas.h)

target_include_directories(doomlike PRIVATE include)
target_link_directories(doomlike PRIVATE libs)
target_link_libraries(doomlike PRIVATE SDL3 SDL3_image)