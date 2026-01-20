#include "engine/core/app.h"
#include "engine/layers/game_layer.h"
#include "engine/layers/level_editor_layer.h"

i32 main(i32 argc, char *argv[]) {
    app_t app = init_app("Doomlike", argc, argv);
    set_layer(&app, &level_editor_layer);
    i32 rc = run_app(&app);
    destroy_app(&app);
    return rc;
}
