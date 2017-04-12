#include <assert.h>
#include "plugin_tiledmap_manip_scene_export_ctx_i.h"
#include "plugin_tiledmap_manip_scene_export_scene_i.h"

int plugin_tiledmap_scene_export(ui_data_mgr_t data_mgr, ui_cache_manager_t cache_mgr, const char * proj_path, error_monitor_t em) {
    plugin_tiledmap_scene_export_ctx_t ctx;
    plugin_tiledmap_scene_export_scene_t scene;
    
    ctx = plugin_tiledmap_scene_export_ctx_create(data_mgr, cache_mgr, proj_path, em);
    if (ctx == NULL) return -1;

    TAILQ_FOREACH(scene, &ctx->m_scenes, m_next_for_ctx) {
        if (plugin_tiledmap_scene_export_scene_do_export(scene) != 0) {
            CPE_ERROR(em, "scene export to %s: export scene %s fail!", proj_path, scene->m_scene_path);
            plugin_tiledmap_scene_export_ctx_free(ctx);
            return -1;
        }
    }

    plugin_tiledmap_scene_export_ctx_free(ctx);
    
    return 0;
}
