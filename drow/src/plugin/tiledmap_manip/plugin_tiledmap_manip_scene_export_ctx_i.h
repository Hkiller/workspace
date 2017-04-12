#ifndef UI_MODEL_MANIP_SCENE_EXPORT_CTX_H
#define UI_MODEL_MANIP_SCENE_EXPORT_CTX_H
#include "cpe/pal/pal_queue.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_module.h"
#include "render/cache/ui_cache_manager.h"
#include "render/cache/ui_cache_pixel_buf.h"
#include "plugin/tiledmap/plugin_tiledmap_data_scene.h"
#include "plugin/tiledmap/plugin_tiledmap_data_layer.h"
#include "plugin/tiledmap/plugin_tiledmap_data_tile.h"

typedef struct plugin_tiledmap_scene_export_ctx * plugin_tiledmap_scene_export_ctx_t;
typedef struct plugin_tiledmap_scene_export_texture * plugin_tiledmap_scene_export_texture_t;
typedef TAILQ_HEAD(plugin_tiledmap_scene_export_texture_list, plugin_tiledmap_scene_export_texture) plugin_tiledmap_scene_export_texture_list_t;
typedef struct plugin_tiledmap_scene_export_scene * plugin_tiledmap_scene_export_scene_t;
typedef TAILQ_HEAD(plugin_tiledmap_scene_export_scene_list, plugin_tiledmap_scene_export_scene) plugin_tiledmap_scene_export_scene_list_t;
typedef struct plugin_tiledmap_scene_export_layer * plugin_tiledmap_scene_export_layer_t;
typedef TAILQ_HEAD(plugin_tiledmap_scene_export_layer_list, plugin_tiledmap_scene_export_layer) plugin_tiledmap_scene_export_layer_list_t;

struct plugin_tiledmap_scene_export_ctx {
    const char * m_proj_path;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    ui_data_mgr_t m_data_mgr;
    ui_cache_manager_t m_cache_mgr;
    plugin_tiledmap_scene_export_scene_list_t m_scenes;
    plugin_tiledmap_scene_export_texture_list_t m_textures;
};

plugin_tiledmap_scene_export_ctx_t
plugin_tiledmap_scene_export_ctx_create(ui_data_mgr_t data_mgr, ui_cache_manager_t cache_mgr, const char * proj_path, error_monitor_t em);
void plugin_tiledmap_scene_export_ctx_free(plugin_tiledmap_scene_export_ctx_t ctx);

#endif
