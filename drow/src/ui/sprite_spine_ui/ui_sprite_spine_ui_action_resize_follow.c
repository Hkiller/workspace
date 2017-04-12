#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "plugin/ui/plugin_ui_animation.h"
#include "plugin/ui/plugin_ui_control.h"
#include "plugin/ui/plugin_ui_control_frame.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_ui/ui_sprite_ui_utils.h"
#include "ui/sprite_ui/ui_sprite_ui_env.h"
#include "ui/sprite_spine_ui/ui_sprite_spine_ui_anim_resize.h"
#include "ui_sprite_spine_ui_action_resize_follow_i.h"

ui_sprite_spine_ui_action_resize_follow_t ui_sprite_spine_ui_action_resize_follow_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_SPINE_UI_ACTION_RESIZE_FOLLOW_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_spine_ui_action_resize_follow_free(ui_sprite_spine_ui_action_resize_follow_t resize_follow) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(resize_follow);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_spine_ui_action_resize_follow_set_control(ui_sprite_spine_ui_action_resize_follow_t resize_follow, const char * control) {
    assert(control);

    if (resize_follow->m_cfg_control) {
        mem_free(resize_follow->m_module->m_alloc, resize_follow->m_cfg_control);
        resize_follow->m_cfg_control = NULL;
    }

    resize_follow->m_cfg_control = cpe_str_mem_dup_trim(resize_follow->m_module->m_alloc, control);
    
    return 0;
}

int ui_sprite_spine_ui_action_resize_follow_set_res(ui_sprite_spine_ui_action_resize_follow_t bind_ress, const char * res) {
    assert(res);

    if (bind_ress->m_cfg_res) {
        mem_free(bind_ress->m_module->m_alloc, bind_ress->m_cfg_res);
        bind_ress->m_cfg_res = NULL;
    }

    bind_ress->m_cfg_res = cpe_str_mem_dup_trim(bind_ress->m_module->m_alloc, res);
    
    return 0;
}

static int ui_sprite_spine_ui_action_resize_follow_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_ui_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_spine_ui_action_resize_follow_t resize_follow = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_ui_env_t ui_env;
    plugin_ui_control_t control;
    plugin_ui_control_frame_t frame;
    plugin_ui_animation_t animation;
    ui_sprite_spine_ui_anim_resize_t anim_resize;
    const char * control_path;
    const char * res;
    
    if (resize_follow->m_cfg_control == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine-ui-resize-follow: control not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    control_path = ui_sprite_fsm_action_check_calc_str(&module->m_dump_buffer, resize_follow->m_cfg_control, fsm_action, NULL, module->m_em);
    if (control_path == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine-ui-resize-follow: calc control %s fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), resize_follow->m_cfg_control);
        return -1;
    }

    control = ui_sprite_ui_find_control_from_action(module->m_ui_module, fsm_action, control_path);
    if (control == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine-ui-resize-follow: control %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), control_path);
        return -1;
    }

    res = ui_sprite_fsm_action_check_calc_str(&module->m_dump_buffer, resize_follow->m_cfg_res, fsm_action, NULL, module->m_em);
    if (res == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine-ui-resize-follow: calc res %s fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), resize_follow->m_cfg_res);
        return -1;
    }

    frame = plugin_ui_control_frame_create_by_res(control, plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_normal, res);
    if (frame == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine-ui-resize-follow: create frame from res %s fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), resize_follow->m_cfg_res);
        return -1;
    }
    
    ui_env = ui_sprite_ui_env_find(ui_sprite_fsm_action_to_world(fsm_action));
    if (ui_env == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine-ui-resize-follow: no ui env",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        plugin_ui_control_frame_free(frame);
        return -1;
    }

    anim_resize = ui_sprite_spine_ui_anim_resize_create(ui_sprite_ui_env_env(ui_env));
    if (anim_resize == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine-ui-resize-follow: create animation fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        plugin_ui_control_frame_free(frame);
        return -1;
    }

    animation = plugin_ui_animation_from_data(anim_resize);
    assert(animation);
    plugin_ui_animation_set_auto_free(animation, 0);
    
    if (ui_sprite_spine_ui_anim_resize_add_frame(anim_resize, frame) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine-ui-resize-follow: add frame to animation fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        plugin_ui_control_frame_free(frame);
        plugin_ui_animation_free(animation);
        return -1;
    }
    
    resize_follow->m_animation_id = plugin_ui_animation_id(animation);
    return ui_sprite_fsm_action_start_update(fsm_action);
}

static void ui_sprite_spine_ui_action_resize_follow_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_spine_ui_module_t module = ctx;
    ui_sprite_spine_ui_action_resize_follow_t resize_follow = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_ui_env_t ui_env = ui_sprite_ui_env_find(ui_sprite_fsm_action_to_world(fsm_action));
    plugin_ui_animation_t animation;

    if (ui_env == NULL) {
        ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine-ui-resize-follow: update: no ui env",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }
    
    animation = plugin_ui_animation_find(ui_sprite_ui_env_env(ui_env), resize_follow->m_animation_id);
    if (animation == NULL || plugin_ui_animation_state(animation) == plugin_ui_animation_state_done) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

static void ui_sprite_spine_ui_action_resize_follow_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_ui_action_resize_follow_t resize_follow = ui_sprite_fsm_action_data(fsm_action);
    plugin_ui_animation_t animation;

    if (resize_follow->m_animation_id) {
        ui_sprite_ui_env_t ui_env = ui_sprite_ui_env_find(ui_sprite_fsm_action_to_world(fsm_action));

        if (ui_env == NULL) {
            animation = plugin_ui_animation_find(ui_sprite_ui_env_env(ui_env), resize_follow->m_animation_id);
            if (animation) {
                plugin_ui_animation_free(animation);
            }
        }
        
        resize_follow->m_animation_id = 0;
    }
}

static int ui_sprite_spine_ui_action_resize_follow_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_ui_action_resize_follow_t resize_follow = ui_sprite_fsm_action_data(fsm_action);
    resize_follow->m_module = ctx;
    resize_follow->m_cfg_control = NULL;
    resize_follow->m_cfg_res = NULL;
    resize_follow->m_animation_id = 0;
    return 0;
}

static void ui_sprite_spine_ui_action_resize_follow_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_ui_module_t modue = ctx;
    ui_sprite_spine_ui_action_resize_follow_t resize_follow = ui_sprite_fsm_action_data(fsm_action);

    if (resize_follow->m_cfg_control) {
        mem_free(modue->m_alloc, resize_follow->m_cfg_control);
        resize_follow->m_cfg_control = NULL;
    }

    if (resize_follow->m_cfg_res) {
        mem_free(modue->m_alloc, resize_follow->m_cfg_res);
        resize_follow->m_cfg_res = NULL;
    }

    assert(resize_follow->m_animation_id == 0);
}

static int ui_sprite_spine_ui_action_resize_follow_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_spine_ui_module_t modue = ctx;    
	ui_sprite_spine_ui_action_resize_follow_t to_resize_follow = ui_sprite_fsm_action_data(to);
	ui_sprite_spine_ui_action_resize_follow_t from_resize_follow = ui_sprite_fsm_action_data(from);

	if (ui_sprite_spine_ui_action_resize_follow_init(to, ctx)) return -1;

    if (from_resize_follow->m_cfg_control) {
        to_resize_follow->m_cfg_control = cpe_str_mem_dup(modue->m_alloc, from_resize_follow->m_cfg_control);
    }

    if (from_resize_follow->m_cfg_res) {
        to_resize_follow->m_cfg_res = cpe_str_mem_dup(modue->m_alloc, from_resize_follow->m_cfg_res);
    }

    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_spine_ui_action_resize_follow_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_spine_ui_module_t module = ctx;
    ui_sprite_spine_ui_action_resize_follow_t resize_follow = ui_sprite_spine_ui_action_resize_follow_create(fsm_state, name);
    const char * str_value;
    
    if (resize_follow == NULL) {
        CPE_ERROR(module->m_em, "%s: create spine-ui-resize-follow action: create fail!", ui_sprite_spine_ui_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "control", NULL))) {
        if (ui_sprite_spine_ui_action_resize_follow_set_control(resize_follow, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create spine-ui-resize-follow action: set control %s fail!",
                ui_sprite_spine_ui_module_name(module), str_value);
            ui_sprite_spine_ui_action_resize_follow_free(resize_follow);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create spine-ui-resize-follow action: control not configured!", ui_sprite_spine_ui_module_name(module));
        ui_sprite_spine_ui_action_resize_follow_free(resize_follow);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "res", NULL))) {
        if (ui_sprite_spine_ui_action_resize_follow_set_res(resize_follow, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create spine-ui-resize-follow action: set res %s fail!",
                ui_sprite_spine_ui_module_name(module), str_value);
            ui_sprite_spine_ui_action_resize_follow_free(resize_follow);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create spine-ui-resize-follow action: res not configured!", ui_sprite_spine_ui_module_name(module));
        ui_sprite_spine_ui_action_resize_follow_free(resize_follow);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(resize_follow);
}

int ui_sprite_spine_ui_action_resize_follow_regist(ui_sprite_spine_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_SPINE_UI_ACTION_RESIZE_FOLLOW_NAME, sizeof(struct ui_sprite_spine_ui_action_resize_follow));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ui enable emitter register: meta create fail",
            ui_sprite_spine_ui_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_spine_ui_action_resize_follow_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_spine_ui_action_resize_follow_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_spine_ui_action_resize_follow_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_spine_ui_action_resize_follow_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_spine_ui_action_resize_follow_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_spine_ui_action_resize_follow_update, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_SPINE_UI_ACTION_RESIZE_FOLLOW_NAME, ui_sprite_spine_ui_action_resize_follow_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_spine_ui_action_resize_follow_unregist(ui_sprite_spine_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_SPINE_UI_ACTION_RESIZE_FOLLOW_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_spine_ui_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_SPINE_UI_ACTION_RESIZE_FOLLOW_NAME);
}

const char * UI_SPRITE_SPINE_UI_ACTION_RESIZE_FOLLOW_NAME = "spine-ui-resize-follow";
