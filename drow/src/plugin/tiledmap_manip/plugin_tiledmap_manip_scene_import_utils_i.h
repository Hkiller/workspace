#ifndef UI_MODEL_MANIP_SCENE_IMPORT_UTILS_H
#define UI_MODEL_MANIP_SCENE_IMPORT_UTILS_H
#include "plugin_tiledmap_manip_scene_import_ctx_i.h"

int plugin_tiledmap_manip_import_load_layer(
    plugin_tiledmap_manip_import_ctx_t ctx,
    ui_cache_pixel_buf_t * pixel_buf, plugin_tiledmap_manip_import_tile_ref_list_t * tile_refs,
    const char * path, const char * name, uint32_t tile_w, uint32_t tile_h);

int plugin_tiledmap_manip_import_build_layer(
    plugin_tiledmap_manip_import_ctx_t ctx, ui_ed_obj_t scene_obj,
    const char * name, plugin_tiledmap_manip_import_tile_ref_list_t * tile_refs,
    int32_t adj_x, int32_t adj_y, uint32_t tile_w, uint32_t tile_h);

#endif
