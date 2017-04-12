#include <assert.h>
#include "gd/app/app_log.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/pal/pal_strings.h"
#include "render/utils/ui_transform.h"
#include "render/utils/ui_rect.h"
#include "plugin/tiledmap/plugin_tiledmap_layer.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_render/ui_sprite_render_env.h"
#include "ui_sprite_tiledmap_action_layer_follow_i.h"
#include "ui_sprite_tiledmap_env_i.h"

ui_sprite_tiledmap_action_layer_follow_t ui_sprite_tiledmap_action_layer_follow_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_TILEDMAP_ACTION_LAYER_FOLLOW_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_tiledmap_action_layer_follow_free(ui_sprite_tiledmap_action_layer_follow_t action_layer_follow) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(action_layer_follow);
    ui_sprite_fsm_action_free(fsm_action);
}

static void ui_sprite_tiledmap_render_pos_update(void * ctx, ui_sprite_render_env_t render_env) {
    ui_sprite_tiledmap_action_layer_follow_t action_layer_follow = ctx;
    ui_sprite_tiledmap_module_t module = action_layer_follow->m_module;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(ui_sprite_fsm_action_from_data(ctx));
    struct ui_rect follow_rect;
    struct ui_rect control_rect;
    ui_transform_t render_trans = ui_sprite_render_env_transform(render_env);
    ui_vector_2_t render_sz = ui_sprite_render_env_size(render_env);
    struct ui_vector_2 render_pos;
    struct ui_transform layer_trans;
    struct ui_vector_2 layer_pos;
    
    assert(action_layer_follow->m_follow_layer);
    assert(action_layer_follow->m_control_layer);

    ui_transform_get_pos_2(render_trans, &render_pos);
    render_pos.x = - render_pos.x;
    render_pos.y = - render_pos.y;
    
    if (plugin_tiledmap_layer_rect(action_layer_follow->m_follow_layer, &follow_rect) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): tiledmap-layer-follow: calc follow layer %s rect fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            plugin_tiledmap_layer_name(action_layer_follow->m_follow_layer));
        return;
    }

    if (plugin_tiledmap_layer_rect(action_layer_follow->m_control_layer, &control_rect) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): tiledmap-layer-follow: calc control layer %s rect fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            plugin_tiledmap_layer_name(action_layer_follow->m_control_layer));
        return;
    }

    layer_trans = *plugin_tiledmap_layer_trans(action_layer_follow->m_control_layer);
    ui_transform_get_pos_2(&layer_trans, &layer_pos);

    if (action_layer_follow->m_cfg_process_x) {
        float follow_w = ui_rect_width(&follow_rect);
        if (follow_w != render_sz->x) {
            float range = follow_w - ui_rect_width(&control_rect);
            float percent = render_pos.x / (follow_w - render_sz->x);
            layer_pos.x = percent * range;
        }
        else {
            layer_pos.x = 0.0f;
        }
    }

    if (action_layer_follow->m_cfg_process_y) {
        float follow_h = ui_rect_height(&follow_rect);
        if (follow_h != render_sz->y) {
            float range = follow_h - ui_rect_height(&control_rect);
            float percent = render_pos.y / (follow_h - render_sz->y);
            layer_pos.y = percent * range;
        }
        else {
            layer_pos.y = 0.0f;
        }
    }

    ui_transform_set_pos_2(&layer_trans, &layer_pos);
    plugin_tiledmap_layer_set_trans(action_layer_follow->m_control_layer, &layer_trans);
}

static int ui_sprite_tiledmap_action_layer_follow_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_tiledmap_module_t module = ctx;
    ui_sprite_tiledmap_action_layer_follow_t action_layer_follow = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_render_env_t render_env = ui_sprite_render_env_find(world);
    ui_sprite_tiledmap_env_t tiledmap_env = ui_sprite_tiledmap_env_find(world);
        
    if (render_env == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): tiledmap-layer-follow: no render env!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (tiledmap_env == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): tiledmap-layer-follow: no tiledmap env!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    
    if (action_layer_follow->m_cfg_control_layer == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): tiledmap-layer-follow: control-layer not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    action_layer_follow->m_control_layer = plugin_tiledmap_layer_find(tiledmap_env->m_env, action_layer_follow->m_cfg_control_layer);
    if (action_layer_follow->m_control_layer == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): tiledmap-layer-follow: follow-layer %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), action_layer_follow->m_cfg_control_layer);
        return -1;
    }

    if (action_layer_follow->m_cfg_follow_layer == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): tiledmap-layer-follow: follow-layer not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    action_layer_follow->m_follow_layer = plugin_tiledmap_layer_find(tiledmap_env->m_env, action_layer_follow->m_cfg_follow_layer);
    if (action_layer_follow->m_follow_layer == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): tiledmap-layer-follow: follow-layer %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), action_layer_follow->m_cfg_follow_layer);
        return -1;
    }
    
    if (ui_sprite_render_env_add_transform_monitor(render_env, ui_sprite_tiledmap_render_pos_update, action_layer_follow) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): tiledmap-layer-follow: add monitor fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    ui_sprite_tiledmap_render_pos_update(action_layer_follow, render_env);

    return 0;
}

static void ui_sprite_tiledmap_action_layer_follow_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_tiledmap_action_layer_follow_t action_layer_follow = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_render_env_t render_env = ui_sprite_render_env_find(world);

    if (render_env) {
        ui_sprite_render_env_remove_transform_monitor(render_env, ui_sprite_tiledmap_render_pos_update, action_layer_follow);
    }
}

static int ui_sprite_tiledmap_action_layer_follow_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_tiledmap_action_layer_follow_t action_layer_follow = ui_sprite_fsm_action_data(fsm_action);
    action_layer_follow->m_module = ctx;
    action_layer_follow->m_cfg_control_layer = NULL;
    action_layer_follow->m_cfg_follow_layer = NULL;
    action_layer_follow->m_cfg_lock_pos.x = 0.0f;
    action_layer_follow->m_cfg_lock_pos.y = 0.0f;
    action_layer_follow->m_cfg_process_x = 0;
    action_layer_follow->m_cfg_process_y = 0;
    action_layer_follow->m_control_layer = NULL;
    action_layer_follow->m_follow_layer = NULL;
    return 0;
}

static void ui_sprite_tiledmap_action_layer_follow_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_tiledmap_module_t module = ctx;
    ui_sprite_tiledmap_action_layer_follow_t action_layer_follow = ui_sprite_fsm_action_data(fsm_action);

    if (action_layer_follow->m_cfg_control_layer) {
        mem_free(module->m_alloc, action_layer_follow->m_cfg_control_layer);
        action_layer_follow->m_cfg_control_layer = NULL;
    }

    if (action_layer_follow->m_cfg_follow_layer) {
        mem_free(module->m_alloc, action_layer_follow->m_cfg_follow_layer);
        action_layer_follow->m_cfg_follow_layer = NULL;
    }
}

static int ui_sprite_tiledmap_action_layer_follow_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_tiledmap_module_t module = ctx;
    ui_sprite_tiledmap_action_layer_follow_t to_action_layer_follow = ui_sprite_fsm_action_data(to);
    ui_sprite_tiledmap_action_layer_follow_t from_action_layer_follow = ui_sprite_fsm_action_data(from);

    if (ui_sprite_tiledmap_action_layer_follow_init(to, ctx)) return -1;

    to_action_layer_follow->m_cfg_process_x = from_action_layer_follow->m_cfg_process_x;
    to_action_layer_follow->m_cfg_process_y = from_action_layer_follow->m_cfg_process_y;
    to_action_layer_follow->m_cfg_lock_pos = from_action_layer_follow->m_cfg_lock_pos;
    to_action_layer_follow->m_cfg_follow_layer = cpe_str_mem_dup(module->m_alloc, from_action_layer_follow->m_cfg_follow_layer);
    to_action_layer_follow->m_cfg_control_layer = cpe_str_mem_dup(module->m_alloc, from_action_layer_follow->m_cfg_control_layer);
    
    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_tiledmap_action_layer_follow_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_tiledmap_module_t module = ctx;
    ui_sprite_tiledmap_action_layer_follow_t action_layer_follow = ui_sprite_tiledmap_action_layer_follow_create(fsm_state, name);
    const char * str_value;
    cfg_t child_cfg;
    
    if (action_layer_follow == NULL) {
        CPE_ERROR(module->m_em, "%s: create tiledmap-layer-follow action: create fail!", ui_sprite_tiledmap_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "control-layer", NULL))) {
        action_layer_follow->m_cfg_control_layer = cpe_str_mem_dup_trim(module->m_alloc, str_value);
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create tiledmap-layer-follow: control-layer not configured!",
            ui_sprite_tiledmap_module_name(module));
        ui_sprite_tiledmap_action_layer_follow_free(action_layer_follow);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "follow-layer", NULL))) {
        action_layer_follow->m_cfg_follow_layer = cpe_str_mem_dup_trim(module->m_alloc, str_value);
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create tiledmap-layer-follow: follow-layer not configured!",
            ui_sprite_tiledmap_module_name(module));
        ui_sprite_tiledmap_action_layer_follow_free(action_layer_follow);
        return NULL;
    }

    if ((child_cfg = cfg_find_cfg(cfg, "lock-pos.x"))) {
        action_layer_follow->m_cfg_process_x = 1;
        action_layer_follow->m_cfg_lock_pos.x = cfg_as_float(child_cfg, 0.0f);
    }

    if ((child_cfg = cfg_find_cfg(cfg, "lock-pos.y"))) {
        action_layer_follow->m_cfg_process_y = 1;
        action_layer_follow->m_cfg_lock_pos.y = cfg_as_float(child_cfg, 0.0f);
    }
    
    return ui_sprite_fsm_action_from_data(action_layer_follow);
}

int ui_sprite_tiledmap_action_layer_follow_regist(ui_sprite_tiledmap_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_TILEDMAP_ACTION_LAYER_FOLLOW_NAME, sizeof(struct ui_sprite_tiledmap_action_layer_follow));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim send event register: meta create fail",
            ui_sprite_tiledmap_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_tiledmap_action_layer_follow_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_tiledmap_action_layer_follow_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_tiledmap_action_layer_follow_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_tiledmap_action_layer_follow_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_tiledmap_action_layer_follow_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_TILEDMAP_ACTION_LAYER_FOLLOW_NAME, ui_sprite_tiledmap_action_layer_follow_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_tiledmap_action_layer_follow_unregist(ui_sprite_tiledmap_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_TILEDMAP_ACTION_LAYER_FOLLOW_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_tiledmap_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_TILEDMAP_ACTION_LAYER_FOLLOW_NAME = "tiledmap-layer-follow";

