#ifndef UI_MODEL_MANIP_SCENE_IMPORT_CTX_H
#define UI_MODEL_MANIP_SCENE_IMPORT_CTX_H
#include "cpe/pal/pal_queue.h"
#include "cpe/cfg/cfg.h"
#include "render/model/ui_data_module.h"
#include "render/model/ui_data_action.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "render/model_ed/ui_ed_obj.h"
#include "render/model_ed/ui_ed_src.h"
#include "render/cache/ui_cache_manager.h"
#include "render/cache/ui_cache_pixel_buf.h"
#include "render/model_manip/model_manip_types.h"

typedef struct plugin_tiledmap_manip_import_ctx * plugin_tiledmap_manip_import_ctx_t;
typedef struct plugin_tiledmap_manip_import_scene * plugin_tiledmap_manip_import_scene_t;
typedef TAILQ_HEAD(plugin_tiledmap_manip_import_scene_list, plugin_tiledmap_manip_import_scene) plugin_tiledmap_manip_import_scene_list_t;
typedef struct plugin_tiledmap_manip_import_layer * plugin_tiledmap_manip_import_layer_t;
typedef TAILQ_HEAD(plugin_tiledmap_manip_import_layer_list, plugin_tiledmap_manip_import_layer) plugin_tiledmap_manip_import_layer_list_t;
typedef struct plugin_tiledmap_manip_import_tile_ref * plugin_tiledmap_manip_import_tile_ref_t;
typedef TAILQ_HEAD(plugin_tiledmap_manip_import_tile_ref_list, plugin_tiledmap_manip_import_tile_ref) plugin_tiledmap_manip_import_tile_ref_list_t;
typedef struct plugin_tiledmap_manip_import_tile * plugin_tiledmap_manip_import_tile_t;
typedef TAILQ_HEAD(plugin_tiledmap_manip_import_tile_list, plugin_tiledmap_manip_import_tile) plugin_tiledmap_manip_import_tile_list_t;
typedef struct plugin_tiledmap_manip_import_merge_ctx * plugin_tiledmap_manip_import_merge_ctx_t;
typedef struct plugin_tiledmap_manip_import_merge_group * plugin_tiledmap_manip_import_merge_group_t;
typedef struct plugin_tiledmap_manip_import_merge_tile * plugin_tiledmap_manip_import_merge_tile_t;
typedef TAILQ_HEAD(plugin_tiledmap_manip_import_merge_tile_list, plugin_tiledmap_manip_import_merge_tile) plugin_tiledmap_manip_import_merge_tile_list_t;
typedef struct plugin_tiledmap_manip_import_tiled_proj * plugin_tiledmap_manip_import_tiled_proj_t;
typedef struct plugin_tiledmap_manip_import_tiled_tileset * plugin_tiledmap_manip_import_tiled_tileset_t;
typedef TAILQ_HEAD(plugin_tiledmap_manip_import_tiled_tileset_list, plugin_tiledmap_manip_import_tiled_tileset) plugin_tiledmap_manip_import_tiled_tileset_list_t;
typedef struct plugin_tiledmap_manip_import_tiled_layer * plugin_tiledmap_manip_import_tiled_layer_t;
typedef TAILQ_HEAD(plugin_tiledmap_manip_import_tiled_layer_list, plugin_tiledmap_manip_import_tiled_layer) plugin_tiledmap_manip_import_tiled_layer_list_t;

struct plugin_tiledmap_manip_import_ctx {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    char m_proj_path[128];
    char m_tile_path[128];
    char m_tileset_path[128];
    uint32_t m_max_width;
    uint32_t m_max_height;
    ui_data_mgr_t m_data_mgr;
    ui_ed_mgr_t m_ed_mgr;
    ui_cache_manager_t m_cache_mgr;
    ui_ed_src_t m_tile_model_src;
    ui_cache_pixel_buf_t m_tile_pixel_buf;
    plugin_tiledmap_manip_import_scene_list_t m_scenes;
    plugin_tiledmap_manip_import_tiled_tileset_list_t m_tilesets;
    uint32_t m_tile_count;
    plugin_tiledmap_manip_import_tile_list_t m_tiles;
};

plugin_tiledmap_manip_import_ctx_t
plugin_tiledmap_manip_import_ctx_load(
    ui_data_mgr_t data_mgr, ui_ed_mgr_t ed_mgr, ui_cache_manager_t cache_mgr, const char * path, error_monitor_t em);

void plugin_tiledmap_manip_import_ctx_free(plugin_tiledmap_manip_import_ctx_t ctx);

#endif
