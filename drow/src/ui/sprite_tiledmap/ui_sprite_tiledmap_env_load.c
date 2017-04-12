#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_cfg.h"
#include "gd/app/app_context.h"
#include "plugin/tiledmap/plugin_tiledmap_layer.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui_sprite_tiledmap_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_world_res_t ui_sprite_tiledmap_env_load(void * ctx, ui_sprite_world_t world, cfg_t cfg) {
    ui_sprite_tiledmap_module_t module = (ui_sprite_tiledmap_module_t)ctx;
    ui_sprite_tiledmap_env_t env = ui_sprite_tiledmap_env_create(module, world);
    struct cfg_it layer_it;
    cfg_t layer_cfg;

    if (env == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create tiledmap_env resource: create tiledmap_env fail!",
            ui_sprite_tiledmap_module_name(module));
        return NULL;
    }

    env->m_debug = cfg_get_uint8(cfg, "debug", 0);

    cfg_it_init(&layer_it, cfg_find_cfg(cfg, "layers"));
    while((layer_cfg = cfg_it_next(&layer_it))) {
        const char * layer_name;
        plugin_tiledmap_layer_t layer;

        layer_name = cfg_get_string(layer_cfg, "name", NULL);
        if (layer_name == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create tiledmap_env resource: layer name not configured!",
                ui_sprite_tiledmap_module_name(module));
            return NULL;
        }

        layer = plugin_tiledmap_layer_create(env->m_env, layer_name);
        if (layer == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create tiledmap_env resource: create layer %s fail!",
                ui_sprite_tiledmap_module_name(module), layer_name);
            return NULL;
        }
    }
    
    return ui_sprite_world_res_from_data(env);
}

#ifdef __cplusplus
}
#endif
