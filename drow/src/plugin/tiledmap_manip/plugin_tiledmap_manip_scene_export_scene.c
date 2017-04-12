#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg.h"
#include "gd/app/app_context.h"
#include "plugin_tiledmap_manip_scene_export_scene_i.h"
#include "plugin_tiledmap_manip_scene_export_layer_i.h"

plugin_tiledmap_scene_export_scene_t
plugin_tiledmap_scene_export_scene_create(plugin_tiledmap_scene_export_ctx_t ctx, cfg_t cfg) {
    plugin_tiledmap_scene_export_scene_t scene;
    ui_data_src_t scene_src;
    struct cfg_it layer_it;
    cfg_t layer_cfg;
    const char * scene_path;
    const char * str_position;
    const char * tiled_proj_path;
    cfg_t tiled_proj_cfg = NULL;

    scene  = mem_calloc(ctx->m_alloc, sizeof(struct plugin_tiledmap_scene_export_scene));
    if (scene == NULL) {
        CPE_ERROR(ctx->m_em, "scene export to %s: alloc export scene fail!", ctx->m_proj_path);
        return NULL;
    }
    scene->m_ctx = ctx;
    TAILQ_INIT(&scene->m_layers);

    scene_path = cfg_get_string(cfg, "path", NULL);
    if (scene_path == NULL) {
        CPE_ERROR(ctx->m_em, "scene export to %s: scene path not configured!", ctx->m_proj_path);
        goto ERROR;
    }
    cpe_str_dup(scene->m_scene_path, sizeof(scene->m_scene_path), scene_path);

    scene_src = ui_data_src_find_by_path(ctx->m_data_mgr, scene->m_scene_path, ui_data_src_type_tiledmap_scene);
    if (scene_src == NULL) {
        CPE_ERROR(ctx->m_em, "scene export to %s: scene %s not exist!", ctx->m_proj_path, scene->m_scene_path);
        goto ERROR;
    }

    if (ui_data_src_check_load_with_usings(scene_src, ctx->m_em) != 0) {
        CPE_ERROR(ctx->m_em, "scene export to %s: scene %s load fail!", ctx->m_proj_path, scene->m_scene_path);
        goto ERROR;
    }

    str_position = cfg_get_string(cfg, "position", NULL);
    if (str_position == NULL) {
        CPE_ERROR(ctx->m_em, "scene export to %s: scene %s: position not configured!", ctx->m_proj_path, scene->m_scene_path);
        goto ERROR;
    }

    if (strcmp(str_position, "bottom-left") == 0) {
        scene->m_scene_export_position = plugin_tiledmap_manip_export_bottom_left;
    }
    else if (strcmp(str_position, "bottom-right") == 0) {
        scene->m_scene_export_position = plugin_tiledmap_manip_export_bottom_right;
    }
    else if (strcmp(str_position, "top-left") == 0) {
        scene->m_scene_export_position = plugin_tiledmap_manip_export_top_left;
    }
    else if (strcmp(str_position, "top-right") == 0) {
        scene->m_scene_export_position = plugin_tiledmap_manip_export_top_right;
    }
    else {
        CPE_ERROR(ctx->m_em, "scene export to %s: scene %s: unknown position %s!", ctx->m_proj_path, scene->m_scene_path, str_position);
        goto ERROR;
    }

    scene->m_data_scene = ui_data_src_product(scene_src);

    cfg_it_init(&layer_it, cfg_find_cfg(cfg, "layers"));
    while((layer_cfg = cfg_it_next(&layer_it))) {
        if (plugin_tiledmap_scene_export_layer_create(scene, layer_cfg) == NULL) {
            goto ERROR;
        }
    }

    if ((tiled_proj_path = cfg_get_string(cfg, "tiled-proj", NULL))) {
        char full_path[128];
        tiled_proj_cfg = cfg_create(ctx->m_alloc);
        if (tiled_proj_cfg == NULL) {
            CPE_ERROR(ctx->m_em, "scene export to %s: tiled proj %s: alloc cfg fail!", ctx->m_proj_path, tiled_proj_path);
            goto ERROR;
        }
    
        snprintf(full_path, sizeof(full_path), "%s/%s", ctx->m_proj_path, tiled_proj_path);
        if (cfg_json_read_file(tiled_proj_cfg, gd_app_vfs_mgr(ui_data_mgr_app(ctx->m_data_mgr)), full_path, cfg_replace, ctx->m_em) != 0) {
            CPE_ERROR(ctx->m_em, "scene export to %s: tiled proj %s: load cfg from %s fail!", ctx->m_proj_path, tiled_proj_path, full_path);
            goto ERROR;
        }
    }

    if (tiled_proj_cfg) cfg_free(tiled_proj_cfg);

    TAILQ_INSERT_TAIL(&ctx->m_scenes, scene, m_next_for_ctx);

    return scene;

ERROR:
    if (tiled_proj_cfg) cfg_free(tiled_proj_cfg);
    plugin_tiledmap_scene_export_scene_free(scene);
    return NULL;
}

void plugin_tiledmap_scene_export_scene_free(plugin_tiledmap_scene_export_scene_t scene) {
    plugin_tiledmap_scene_export_ctx_t ctx = scene->m_ctx;

    while(!TAILQ_EMPTY(&scene->m_layers)) {
        plugin_tiledmap_scene_export_layer_free(TAILQ_FIRST(&scene->m_layers));
    }

    TAILQ_REMOVE(&ctx->m_scenes, scene, m_next_for_ctx);

    mem_free(ctx->m_alloc, scene);
}

int plugin_tiledmap_scene_export_scene_do_export(plugin_tiledmap_scene_export_scene_t scene) {
    plugin_tiledmap_scene_export_layer_t layer;

    TAILQ_FOREACH(layer, &scene->m_layers, m_next_for_scene) {
        if (plugin_tiledmap_scene_export_layer_do_export(layer) != 0) return -1;
    }
    
    return 0;
}

