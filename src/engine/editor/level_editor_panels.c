#include "level_editor_panels.h"

#include <float.h>

#include "../core/renderer.h"
#include "nuklear_init.h"
#include "../core/app.h"

void draw_sector_edit_panel(sector_t *sector) {
    struct nk_context *ctx = g_nuklear_instance->ctx;

    /* GUI */
    if (nk_begin(ctx, "EDIT SECTOR", nk_rect(50, 50, 230, 250),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
        NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
    {
        nk_layout_row_dynamic(ctx, 25, 1);
        char buf[32];
        sprintf(buf, "Sector : %d", (int)(sector - (sector_t*)g_state->level.sections.data));
        nk_label(ctx, buf, NK_TEXT_ALIGN_CENTERED);
        nk_property_float(ctx, "Floor Height", 0, &sector->floor, sector->ceil, 0.01, 1);
        nk_property_float(ctx, "Ceiling Height", sector->floor, &sector->ceil, 100, 0.01, 1);
    }
    nk_end(ctx);
}

void draw_vertex_edit_panel(vec2 *vec) {
    struct nk_context *ctx = g_nuklear_instance->ctx;

    if (nk_begin(ctx, "EDIT VERTEX", nk_rect(50, 50, 230,75),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
        NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
    {
        float x = (*vec)[0], y = (*vec)[1];
        nk_layout_row_dynamic(ctx, 25, 2);
        nk_property_float(ctx, "X", FLT_MIN, &x, FLT_MAX, 0.01, 1);
        nk_property_float(ctx, "Y", FLT_MIN, &y, FLT_MAX, 0.01, 1);
        (*vec)[0] = x;
        (*vec)[1] = y;
    }
    nk_end(ctx);
}
