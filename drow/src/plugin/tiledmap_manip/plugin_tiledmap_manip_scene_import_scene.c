#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg.h"
#include "plugin_tiledmap_manip_scene_import_scene_i.h"
#include "plugin_tiledmap_manip_scene_import_layer_i.h"
#include "plugin_tiledmap_manip_scene_import_tiled_proj_i.h"

plugin_tiledmap_manip_import_scene_t
plugin_tiledmap_manip_import_scene_create(plugin_tiledmap_manip_import_ctx_t ctx, cfg_t cfg) {
    plugin_tiledmap_manip_import_scene_t scene;
    struct cfg_it layer_it;
    cfg_t layer_cfg;
    const char * scene_path;
    const char * str_position;
    const char * tiled_proj_path;

    scene = mem_calloc(ctx->m_alloc, sizeof(struct plugin_tiledmap_manip_import_scene));
    if (scene == NULL) {
        CPE_ERROR(ctx->m_em, "scene import from %s: alloc scene fail!", ctx->m_proj_path);
        return NULL;
    }

    scene->m_ctx = ctx;
    TAILQ_INIT(&scene->m_layers);
    TAILQ_INSERT_TAIL(&ctx->m_scenes, scene, m_next);

    /*begin load*/
    scene_path = cfg_get_string(cfg, "path", NULL);
    if (scene_path == NULL) {
        CPE_ERROR(ctx->m_em, "scene import from %s: load scene: path not configured!", ctx->m_proj_path);
        plugin_tiledmap_manip_import_scene_free(scene);
        return NULL;
    }
    cpe_str_dup(scene->m_path, sizeof(scene->m_path), scene_path);

    str_position = cfg_get_string(cfg, "position", NULL);
    if (str_position == NULL) {
        CPE_ERROR(ctx->m_em, "scene import from %s: load scene: position not configured!", ctx->m_proj_path);
        plugin_tiledmap_manip_import_scene_free(scene);
        return NULL;
    }
    
    if (strcmp(str_position, "bottom-left") == 0) {
        scene->m_position = plugin_tiledmap_manip_import_bottom_left;
    }
    else if (strcmp(str_position, "bottom-right") == 0) {
        scene->m_position = plugin_tiledmap_manip_import_bottom_right;
    }
    else if (strcmp(str_position, "top-left") == 0) {
        scene->m_position = plugin_tiledmap_manip_import_top_left;
    }
    else if (strcmp(str_position, "top-right") == 0) {
        scene->m_position = plugin_tiledmap_manip_import_top_right;
    }
    else {
        CPE_ERROR(ctx->m_em, "scene import from %s: load scene: unknown position %s!", ctx->m_proj_path, str_position);
        plugin_tiledmap_manip_import_scene_free(scene);
        return NULL;
    }

    /*load layers */
    cfg_it_init(&layer_it, cfg_find_cfg(cfg, "layers"));
    while((layer_cfg = cfg_it_next(&layer_it))) {
        if (plugin_tiledmap_manip_import_layer_create(scene, layer_cfg) == NULL) {
            plugin_tiledmap_manip_import_scene_free(scene);
            return NULL;
        }
    }

    if ((tiled_proj_path = cfg_get_string(cfg, "tiled-proj", NULL))) {
        scene->m_tiled_proj = plugin_tiledmap_manip_import_tiled_proj_create(scene, tiled_proj_path);
        if (scene->m_tiled_proj == NULL) {
            plugin_tiledmap_manip_import_scene_free(scene);
            return NULL;
        }
    }
    
    return scene;
}

void plugin_tiledmap_manip_import_scene_free(plugin_tiledmap_manip_import_scene_t scene) {
    plugin_tiledmap_manip_import_ctx_t ctx = scene->m_ctx;

    while(!TAILQ_EMPTY(&scene->m_layers)) {
        plugin_tiledmap_manip_import_layer_free(TAILQ_FIRST(&scene->m_layers));
    }

    if (scene->m_tiled_proj) {
        plugin_tiledmap_manip_import_tiled_proj_free(scene->m_tiled_proj);
        scene->m_tiled_proj = NULL;
    }

    TAILQ_REMOVE(&ctx->m_scenes, scene, m_next);
    mem_free(ctx->m_alloc, scene);
}
