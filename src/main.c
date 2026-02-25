#include <libunwind-x86_64.h>
#include <sys/time.h>

#include "engine/core/app.h"
#include "engine/core/ecs/ecs.h"
#include "engine/jobs/job.h"
#include "engine/layers/game_layer.h"
#include "engine/layers/level_editor_layer.h"

i32 main(i32 argc, char *argv[]) {
    initialize_thread_pool();
    app_t app = init_app("Doomlike", argc, argv);
    g_nuklear_instance = &app.nk;
    set_layer(&app, &level_editor_layer);
    i32 rc = run_app(&app);
    destroy_app(&app);
    destroy_thread_pool();
    return rc;
}
