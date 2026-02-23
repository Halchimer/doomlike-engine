cmake_minimum_required(VERSION 4.1)
project(doomlike C)

set(CMAKE_C_STANDARD 23)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON  CACHE BOOL "" FORCE)

set(SDLIMAGE_SHARED OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_STATIC ON  CACHE BOOL "" FORCE)

add_subdirectory(libs/SDL)
add_subdirectory(libs/SDL_image)

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
        src/engine/core/ecs/components/camera.c
        src/engine/core/ecs/components/camera.h
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
        src/engine/core/texture_atlas.h
        src/engine/core/ecs/ecs.c
        src/engine/core/ecs/ecs.h
        src/engine/core/ecs/component.c
        src/engine/core/ecs/component.h
        src/engine/core/ecs/entity.h
        src/engine/component_ids.h
        src/engine/core/ecs/components/transform.c
        src/engine/core/ecs/components/transform.h
        src/engine/core/ecs/components/sprite.c
        src/engine/core/ecs/components/sprite.h
        src/engine/core/ecs/components/billboard.c
        src/engine/core/ecs/components/billboard.h
        src/engine/core/ecs/components/velocity.c
        src/engine/core/ecs/components/velocity.h
        src/engine/core/ecs/components/level_collider.c
        src/engine/core/ecs/components/level_collider.h
        src/engine/core/ecs/components/sector_component.c
        src/engine/core/ecs/components/sector_component.h
        src/engine/asset/asset_manager.c
        src/engine/asset/asset_manager.h
        src/engine/lighting/lightmap.c
        src/engine/lighting/lightmap.h
        src/engine/core/ecs/components/light.c
        src/engine/core/ecs/components/light.h
        src/engine/jobs/job.c
        src/engine/jobs/job.h
        src/engine/jobs/thread_pool.c
        src/engine/jobs/thread_pool.h
        src/engine/layers/debug_info_layer.c
        src/engine/layers/debug_info_layer.h)

target_link_libraries(doomlike PRIVATE SDL3::SDL3-static SDL3_image::SDL3_image)
target_include_directories(doomlike PRIVATE include)
