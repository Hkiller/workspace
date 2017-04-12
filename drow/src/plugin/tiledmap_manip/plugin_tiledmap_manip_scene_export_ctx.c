#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/file.h"
#include "cpe/cfg/cfg.h"
#include "cpe/utils/error.h"
#include "gd/app/app_context.h"
#include "plugin_tiledmap_manip_scene_export_ctx_i.h"
#include "plugin_tiledmap_manip_scene_export_scene_i.h"
#include "plugin_tiledmap_manip_scene_export_texture_i.h"

plugin_tiledmap_scene_export_ctx_t
plugin_tiledmap_scene_export_ctx_create(ui_data_mgr_t data_mgr, ui_cache_manager_t cache_mgr, const char * proj_path, error_monitor_t em) {
    plugin_tiledmap_scene_export_ctx_t ctx = NULL;
    char cfg_path[128];
    cfg_t cfg = NULL;
    struct cfg_it scene_it;
    cfg_t scene_cfg;

    ctx = mem_calloc(NULL, sizeof(struct plugin_tiledmap_scene_export_ctx));
    if (ctx == NULL) {
        CPE_ERROR(em, "plugin_tiledmap_scene_export_ctx_create: alloc fail");
        return NULL;
    }
    
    ctx->m_em = em;
    ctx->m_alloc = NULL;
    ctx->m_proj_path = proj_path;
    ctx->m_data_mgr = data_mgr;
    ctx->m_cache_mgr = cache_mgr;
    
    TAILQ_INIT(&ctx->m_textures);
    TAILQ_INIT(&ctx->m_scenes);

    cfg = cfg_create(NULL);
    if (cfg == NULL) {
        CPE_ERROR(em, "scene export to %s: create cfg fail!", proj_path);
        goto ERROR;
    }
    
    snprintf(cfg_path, sizeof(cfg_path), "%s/proj.yml", proj_path);
    if (cfg_yaml_read_file(cfg, gd_app_vfs_mgr(ui_data_mgr_app(data_mgr)), cfg_path, cfg_merge_use_new, em) != 0) {
        CPE_ERROR(em, "scene export to %s: load cfg to '%s' fail!", proj_path, cfg_path);
        goto ERROR;
    }

    cfg_it_init(&scene_it, cfg_find_cfg(cfg, "scenes"));
    while((scene_cfg = cfg_it_next(&scene_it))) {
        if (plugin_tiledmap_scene_export_scene_create(ctx, scene_cfg) == NULL) {
            goto ERROR;
        }
    }
    
    return ctx;

ERROR:
    plugin_tiledmap_scene_export_ctx_free(ctx);
    if (cfg) cfg_free(cfg);
    return NULL;
}

void plugin_tiledmap_scene_export_ctx_free(plugin_tiledmap_scene_export_ctx_t ctx) {
    while(!TAILQ_EMPTY(&ctx->m_scenes)) {
        plugin_tiledmap_scene_export_scene_free(TAILQ_FIRST(&ctx->m_scenes));
    }

    while(!TAILQ_EMPTY(&ctx->m_textures)) {
        plugin_tiledmap_scene_export_texutre_free(TAILQ_FIRST(&ctx->m_textures));
    }

    mem_free(ctx->m_alloc, ctx);
}
