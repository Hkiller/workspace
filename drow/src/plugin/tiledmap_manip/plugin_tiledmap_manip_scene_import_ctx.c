#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_mgr.h"
#include "plugin_tiledmap_manip_scene_import_ctx_i.h"
#include "plugin_tiledmap_manip_scene_import_scene_i.h"
#include "plugin_tiledmap_manip_scene_import_tiled_tileset_i.h"
#include "plugin_tiledmap_manip_scene_import_tile_i.h"

plugin_tiledmap_manip_import_ctx_t
plugin_tiledmap_manip_import_ctx_load(
    ui_data_mgr_t data_mgr, ui_ed_mgr_t ed_mgr, ui_cache_manager_t cache_mgr, const char * path, error_monitor_t em)
{
    char cfg_path[128];
    cfg_t cfg;
    mem_allocrator_t alloc = NULL;
    plugin_tiledmap_manip_import_ctx_t ctx;
    struct cfg_it scene_it;
    cfg_t scene_cfg;
    const char * tile_path;
    const char * tileset_path;

    ctx = mem_calloc(alloc, sizeof(struct plugin_tiledmap_manip_import_ctx));
    if (ctx == NULL) {
        CPE_ERROR(em, "scene import from %s: create cfg fail!", path);
        return NULL;
    }

    ctx->m_alloc = alloc;
    ctx->m_em = em;
    cpe_str_dup(ctx->m_proj_path, sizeof(ctx->m_proj_path), path);
    ctx->m_data_mgr = data_mgr;
    ctx->m_ed_mgr = ed_mgr;
    ctx->m_cache_mgr = cache_mgr;
    TAILQ_INIT(&ctx->m_scenes);
    TAILQ_INIT(&ctx->m_tilesets);
    TAILQ_INIT(&ctx->m_tiles);

    cfg = cfg_create(alloc);
    if (cfg == NULL) {
        CPE_ERROR(em, "scene import from %s: create cfg fail!", path);
        plugin_tiledmap_manip_import_ctx_free(ctx);
        return NULL;
    }
    
    snprintf(cfg_path, sizeof(cfg_path), "%s/proj.yml", path);
    if (cfg_yaml_read_file(cfg, gd_app_vfs_mgr(ui_data_mgr_app(data_mgr)), cfg_path, cfg_merge_use_new, em) != 0) {
        CPE_ERROR(em, "scene import from %s: load cfg from '%s' fail!", path, cfg_path);
        plugin_tiledmap_manip_import_ctx_free(ctx);
        cfg_free(cfg);
        return NULL;
    }

    tile_path = cfg_get_string(cfg, "tile", NULL);
    if (tile_path == NULL) {
        CPE_ERROR(em, "scene import from %s: tile not configured!", path);
        plugin_tiledmap_manip_import_ctx_free(ctx);
        cfg_free(cfg);
        return NULL;
    }
    cpe_str_dup(ctx->m_tile_path, sizeof(ctx->m_tile_path), tile_path);

    tileset_path = cfg_get_string(cfg, "tileset", NULL);
    if (tileset_path) {
        cpe_str_dup(ctx->m_tileset_path, sizeof(ctx->m_tileset_path), tileset_path);
    }
    
    ctx->m_max_width = cfg_get_uint32(cfg, "max-size.width", 0);
    if (ctx->m_max_width == 0) {
        CPE_ERROR(em, "scene import from %s: max-size.width not configured!", path);
        plugin_tiledmap_manip_import_ctx_free(ctx);
        cfg_free(cfg);
        return NULL;
    }

    ctx->m_max_height = cfg_get_uint32(cfg, "max-size.height", 0);
    if (ctx->m_max_height == 0) {
        CPE_ERROR(em, "scene import from %s: max-size.height not configured!", path);
        plugin_tiledmap_manip_import_ctx_free(ctx);
        cfg_free(cfg);
        return NULL;
    }
    
    cfg_it_init(&scene_it, cfg_find_cfg(cfg, "scenes"));
    while((scene_cfg = cfg_it_next(&scene_it))) {
        if (plugin_tiledmap_manip_import_scene_create(ctx, scene_cfg) == NULL) {
            CPE_ERROR(em, "scene import from %s: create scene fail!", path);
            plugin_tiledmap_manip_import_ctx_free(ctx);
            cfg_free(cfg);
            return NULL;
        }
    }

    cfg_free(cfg);
    return ctx;
}

void plugin_tiledmap_manip_import_ctx_free(plugin_tiledmap_manip_import_ctx_t ctx) {
    while(!TAILQ_EMPTY(&ctx->m_tiles)) {
        plugin_tiledmap_manip_import_tile_free(TAILQ_FIRST(&ctx->m_tiles));
    }

    while(!TAILQ_EMPTY(&ctx->m_scenes)) {
        plugin_tiledmap_manip_import_scene_free(TAILQ_FIRST(&ctx->m_scenes));
    }

    while(!TAILQ_EMPTY(&ctx->m_tilesets)) {
        plugin_tiledmap_manip_import_tiled_tileset_free(TAILQ_FIRST(&ctx->m_tilesets));
    }

    if (ctx->m_tile_pixel_buf) {
        ui_cache_pixel_buf_free(ctx->m_tile_pixel_buf);
        ctx->m_tile_pixel_buf = NULL;
    }

    mem_free(ctx->m_alloc, ctx);
}
