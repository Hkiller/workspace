#include <assert.h>
#include "cpe/utils/file.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_mgr.h"
#include "plugin_tiledmap_manip_scene_import_ctx_i.h"
#include "plugin_tiledmap_manip_scene_import_scene_i.h"
#include "plugin_tiledmap_manip_scene_import_layer_i.h"
#include "plugin_tiledmap_manip_scene_import_tile_i.h"
#include "plugin_tiledmap_manip_scene_import_merge_i.h"
#include "plugin_tiledmap_manip_scene_import_tiled_proj_i.h"
#include "plugin_tiledmap_manip_scene_import_tiled_layer_i.h"
#include "plugin_tiledmap_manip_scene_import_tiled_tileset_i.h"

int plugin_tiledmap_scene_import(
    ui_data_mgr_t data_mgr, ui_ed_mgr_t ed_mgr, ui_cache_manager_t cache_mgr,
    const char * proj_path, error_monitor_t em)
{
    plugin_tiledmap_manip_import_ctx_t ctx;
    plugin_tiledmap_manip_import_scene_t scene;
    plugin_tiledmap_manip_import_layer_t layer;
    plugin_tiledmap_manip_import_tiled_tileset_t tileset;

    ctx = plugin_tiledmap_manip_import_ctx_load(data_mgr, ed_mgr, cache_mgr, proj_path, em);
    if (ctx == NULL) return -1;

    /*加载所有Tileset图片 */
    TAILQ_FOREACH(tileset, &ctx->m_tilesets, m_next_for_ctx) {
        if (plugin_tiledmap_manip_import_tiled_tileset_load(tileset) != 0) {
            plugin_tiledmap_manip_import_ctx_free(ctx);
            return -1;
        }
    }

    /*加载所有场景图片 */
    TAILQ_FOREACH(scene, &ctx->m_scenes, m_next) {
        TAILQ_FOREACH(layer, &scene->m_layers, m_next) {
            if (plugin_tiledmap_manip_import_layer_load(layer) != 0) {
                plugin_tiledmap_manip_import_ctx_free(ctx);
                return -1;
            }
        }

        /*加载所有场景Tiled层 */
        if (scene->m_tiled_proj) {
            plugin_tiledmap_manip_import_tiled_layer_t tiled_layer;

            TAILQ_FOREACH(tiled_layer, &scene->m_tiled_proj->m_layers, m_next) {
                if (plugin_tiledmap_manip_import_tiled_layer_load(tiled_layer) != 0) {
                    plugin_tiledmap_manip_import_ctx_free(ctx);
                    return -1;
                }
            }
        }
    }

    /*Tile块合并 */
    if (plugin_tiledmap_manip_import_merge(ctx) != 0) {
        plugin_tiledmap_manip_import_ctx_free(ctx);
        return -1;
    }

    /*Tile块生成Module */
    if (plugin_tiledmap_manip_import_tile_build_module(ctx) != 0) {
        plugin_tiledmap_manip_import_ctx_free(ctx);
        return -1;
    }

    /*构建Tileset */
    if (plugin_tiledmap_manip_import_tiled_tileset_build(ctx) != 0) {
        plugin_tiledmap_manip_import_ctx_free(ctx);
        return -1;
    }

    /*构建所有Map */
    TAILQ_FOREACH(scene, &ctx->m_scenes, m_next) {
        if (plugin_tiledmap_manip_import_scene_build_tiledmap(scene) != 0) {
            plugin_tiledmap_manip_import_ctx_free(ctx);
            return -1;
        }
    }

    /*保存图片 */
    do {
        char pixel_file[128];
        char * p;
        snprintf(
            pixel_file, sizeof(pixel_file), "%s/%s.png",
            ui_data_src_data(ui_data_mgr_src_root(ctx->m_data_mgr)),
            ctx->m_tile_path);
        p = strrchr(pixel_file, '/');
        assert(p);

        *p = 0;
        if (dir_mk_recursion(pixel_file, DIR_DEFAULT_MODE, ctx->m_em, ctx->m_alloc) != 0) {
            CPE_ERROR(em, "create pixel file dir %s fail!", pixel_file);
            plugin_tiledmap_manip_import_ctx_free(ctx);
            return -1;
        }
        *p = '/';

        if (ui_cache_pixel_buf_save_to_file(ctx->m_tile_pixel_buf, pixel_file, ctx->m_em, ctx->m_alloc) != 0) {
            CPE_ERROR(em, "create pixel file %s fail!", pixel_file);
            plugin_tiledmap_manip_import_ctx_free(ctx);
            return -1;
        }
    } while(0);

    plugin_tiledmap_manip_import_ctx_free(ctx);
    return 0;
}
