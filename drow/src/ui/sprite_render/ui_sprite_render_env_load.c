#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui_sprite_render_env_i.h"
#include "ui_sprite_render_layer_i.h"
#include "ui_sprite_render_anim_i.h"

static int ui_sprite_render_env_load_layer(
    ui_sprite_render_module_t module, ui_sprite_world_t world, ui_sprite_render_layer_t layer, cfg_t cfg);

ui_sprite_world_res_t ui_sprite_render_env_res_load(void * ctx, ui_sprite_world_t world, cfg_t cfg) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_env_t env = ui_sprite_render_env_create(module, world);
    struct cfg_it layers_it;
    cfg_t layer_cfg;
    ui_sprite_render_layer_t before_layer = NULL;

    if (env == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_render_env_res_load: create env fail!");
        return NULL;
    }
    
    ui_sprite_render_env_set_debug(env, cfg_get_uint8(cfg, "debug", env->m_debug));
    ui_sprite_render_env_set_update_priority(env, cfg_get_uint8(cfg, "update-priority", 0));

    env->m_design_size.x = cfg_get_float(cfg, "design-size.x", 0.0f);
    env->m_design_size.y = cfg_get_float(cfg, "design-size.y", 0.0f);
    
    cfg_it_init(&layers_it, cfg_find_cfg(cfg, "layers"));
    while((layer_cfg = cfg_it_next(&layers_it))) {
        const char * layer_name = cfg_is_value(layer_cfg) ? cfg_as_string(layer_cfg, "") : cfg_name(cfg_child_only(layer_cfg));
        if (strcmp(layer_name, "default") == 0) {
            before_layer = env->m_default_layer;
            break;
        }
    }

    cfg_it_init(&layers_it, cfg_find_cfg(cfg, "layers"));
    while((layer_cfg = cfg_it_next(&layers_it))) {
        const char * layer_name;
        
        if (!cfg_is_value(layer_cfg)) {
            layer_cfg = cfg_child_only(layer_cfg);
            layer_name = cfg_name(layer_cfg);
        }
        else {
            layer_name = cfg_as_string(layer_cfg, NULL); 
            layer_cfg = NULL;
        }

        if (strcmp(layer_name, "default") == 0) {
            if (ui_sprite_render_env_load_layer(module, world, before_layer, layer_cfg) != 0) {
                ui_sprite_render_env_free(world);
                return NULL;
            }
            before_layer = NULL;
        }
        else {
            ui_sprite_render_layer_t layer = ui_sprite_render_layer_create(env, before_layer, layer_name);
            if (layer == NULL) {
                CPE_ERROR(module->m_em, "ui_sprite_render_env_load: create layer %s fail!", layer_name);
                ui_sprite_render_env_free(world);
                return NULL;
            }

            if (ui_sprite_render_env_load_layer(module, world, layer, layer_cfg) != 0) {
                ui_sprite_render_env_free(world);
                return NULL;
            }
        }
    }

    return ui_sprite_world_res_from_data(env);
}

static int ui_sprite_render_env_load_layer(
    ui_sprite_render_module_t module, ui_sprite_world_t world, ui_sprite_render_layer_t layer, cfg_t cfg)
{
    struct cfg_it init_node_it;
    cfg_t init_cfg;
    
    layer->m_is_free = cfg_get_uint8(cfg, "is-free", layer->m_is_free);

    cfg_it_init(&init_node_it, cfg_find_cfg(cfg, "init"));
    while((init_cfg = cfg_it_next(&init_node_it))) {
        ui_sprite_render_anim_t anim;
        const char * res;

        res = cfg_as_string(init_cfg, NULL);
        if (res == NULL) {
            CPE_ERROR(
                module->m_em, "ui_sprite_render_env_load: layer %s: create init node: res format error!",
                layer->m_name);
            return -1;
        }

        anim = ui_sprite_render_anim_create_by_res(layer, res, NULL, NULL);
        if (anim == NULL) {
            CPE_ERROR(
                module->m_em, "ui_sprite_render_env_load: layer %s: create init node: create anim fail!",
                layer->m_name);
            return -1;
        }
    }

    return 0;
}
