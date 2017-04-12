#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_cfg.h"
#include "gd/app/app_context.h"
#include "render/utils/ui_vector_2.h"
#include "plugin/scrollmap/plugin_scrollmap_env.h"
#include "plugin/scrollmap/plugin_scrollmap_layer.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui_sprite_scrollmap_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_world_res_t ui_sprite_scrollmap_env_load(void * ctx, ui_sprite_world_t world, cfg_t cfg) {
    ui_sprite_scrollmap_module_t module = (ui_sprite_scrollmap_module_t)ctx;
    ui_sprite_scrollmap_env_t env;
    struct cfg_it layer_it;
    cfg_t layer_cfg;
    const char * str_value;
    plugin_scrollmap_moving_way_t moving_way;
    ui_vector_2 base_size;
    
    str_value = cfg_get_string(cfg, "moving-way", NULL);
    if (str_value == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create scrollmap_env resource: create scrollmap_env fail!",
            ui_sprite_scrollmap_module_name(module));
        return NULL;
    }

    if (strcmp(str_value, "down") == 0) {
        moving_way = plugin_scrollmap_moving_down;
    }
    else if (strcmp(str_value, "up") == 0) {
        moving_way = plugin_scrollmap_moving_up;
    }
    else if (strcmp(str_value, "left") == 0) {
        moving_way = plugin_scrollmap_moving_left;
    }
    else if (strcmp(str_value, "right") == 0) {
        moving_way = plugin_scrollmap_moving_right;
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create scrollmap_env resource: unknown moving-way %s!",
            ui_sprite_scrollmap_module_name(module), str_value);
        return NULL;
    }

    base_size.x = cfg_get_uint32(cfg, "base-size.x", 0);
    if (base_size.x <= 0.0f) {
        CPE_ERROR(
            module->m_em, "%s: create scrollmap_env resource: base-size.x error!",
            ui_sprite_scrollmap_module_name(module));
        return NULL;
    }
    
    base_size.y = cfg_get_uint32(cfg, "base-size.y", 0);
    if (base_size.y <= 0.0f) {
        CPE_ERROR(
            module->m_em, "%s: create scrollmap_env resource: base-size.y error!",
            ui_sprite_scrollmap_module_name(module));
        return NULL;
    }
    
    env = ui_sprite_scrollmap_env_create(module, world, moving_way, &base_size);
    if (env == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create scrollmap_env resource: create scrollmap_env fail!",
            ui_sprite_scrollmap_module_name(module));
        return NULL;
    }

    if (ui_sprite_scrollmap_env_set_update_priority(env, cfg_get_int8(cfg, "update-priority", 0)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create scrollmap_env resource: set update priority %d fail!",
            ui_sprite_scrollmap_module_name(module), cfg_get_int8(cfg, "update-priority", 0));
        ui_sprite_scrollmap_env_free(world);
        return NULL;
    }
    
    env->m_debug = cfg_get_uint8(cfg, "debug", 0);
    plugin_scrollmap_env_set_debug(env->m_env, env->m_debug);

    if ((str_value = cfg_get_string(cfg, "resize-policy-x", NULL))) {
        if (strcmp(str_value, "percent") == 0) {
            plugin_scrollmap_env_set_resize_policy_x(env->m_env, plugin_scrollmap_resize_policy_percent);
        }
        else if (strcmp(str_value, "fix") == 0) {
            plugin_scrollmap_env_set_resize_policy_x(env->m_env, plugin_scrollmap_resize_policy_fix);
        }
        else {
            CPE_ERROR(
                module->m_em, "%s: create scrollmap_env resource: unknown resize-policy-x %s!",
                ui_sprite_scrollmap_module_name(module), str_value);
            ui_sprite_scrollmap_env_free(world);
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "resize-policy-y", NULL))) {
        if (strcmp(str_value, "percent") == 0) {
            plugin_scrollmap_env_set_resize_policy_y(env->m_env, plugin_scrollmap_resize_policy_percent);
        }
        else if (strcmp(str_value, "fix") == 0) {
            plugin_scrollmap_env_set_resize_policy_y(env->m_env, plugin_scrollmap_resize_policy_fix);
        }
        else {
            CPE_ERROR(
                module->m_em, "%s: create scrollmap_env resource: unknown resize-policy-y %s!",
                ui_sprite_scrollmap_module_name(module), str_value);
            ui_sprite_scrollmap_env_free(world);
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "runing-size-policy", NULL))) {
        if (strcmp(str_value, "no-adj") == 0) {
            ui_sprite_scrollmap_env_set_runtime_size_policy(env, ui_sprite_scrollmap_runtime_size_no_adj);
        }
        else if (strcmp(str_value, "fix-x") == 0) {
            ui_sprite_scrollmap_env_set_runtime_size_policy(env, ui_sprite_scrollmap_runtime_size_fix_x);
        }
        else if (strcmp(str_value, "fix-y") == 0) {
            ui_sprite_scrollmap_env_set_runtime_size_policy(env, ui_sprite_scrollmap_runtime_size_fix_y);
        }
        else if (strcmp(str_value, "resize") == 0) {
            ui_sprite_scrollmap_env_set_runtime_size_policy(env, ui_sprite_scrollmap_runtime_size_resize);
        }
        else {
            CPE_ERROR(
                module->m_em, "%s: create scrollmap_env resource: unknown runing-size-policy %s!",
                ui_sprite_scrollmap_module_name(module), str_value);
            ui_sprite_scrollmap_env_free(world);
            return NULL;
        }
    }
    
    cfg_it_init(&layer_it, cfg_find_cfg(cfg, "layers"));
    while((layer_cfg = cfg_it_next(&layer_it))) {
        const char * layer_name;
        plugin_scrollmap_layer_t layer;

        layer_name = cfg_get_string(layer_cfg, "name", NULL);
        if (layer_name == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create scrollmap_env resource: layer name not configured!",
                ui_sprite_scrollmap_module_name(module));
            ui_sprite_scrollmap_env_free(world);
            return NULL;
        }

        layer = plugin_scrollmap_layer_create(env->m_env, layer_name);
        if (layer == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create scrollmap_env resource: create layer %s fail!",
                ui_sprite_scrollmap_module_name(module), layer_name);
            ui_sprite_scrollmap_env_free(world);
            return NULL;
        }
    }
    
    return ui_sprite_world_res_from_data(env);
}

#ifdef __cplusplus
}
#endif
