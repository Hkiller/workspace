#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "gd/app/app_context.h"
#include "plugin/ui/plugin_ui_animation.h"
#include "plugin/ui/plugin_ui_control.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_ui/ui_sprite_ui_utils.h"
#include "ui_sprite_ui_action_control_anim_i.h"
#include "ui_sprite_ui_env_i.h"

ui_sprite_ui_action_control_anim_t ui_sprite_ui_action_control_anim_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_UI_ACTION_CONTROL_ANIM_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_ui_action_control_anim_free(ui_sprite_ui_action_control_anim_t action_control_anim) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(action_control_anim);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_ui_action_control_anim_set_control(ui_sprite_ui_action_control_anim_t control_anim, const char * control) {
    if (control_anim->m_cfg_control) {
        mem_free(control_anim->m_module->m_alloc, control_anim->m_cfg_control);
    }

    if (control) {
        control_anim->m_cfg_control = cpe_str_mem_dup_trim(control_anim->m_module->m_alloc, control);
        if (control_anim->m_cfg_control == NULL) {
            CPE_ERROR(control_anim->m_module->m_em, "ui_sprite_ui_action_control_anim_set_control: dup str fail!");
            return -1;
        }
    }
    else {
        control_anim->m_cfg_control = NULL;
    }

    return 0;
}

const char * ui_sprite_ui_action_control_anim_control(ui_sprite_ui_action_control_anim_t control_anim) {
    return control_anim->m_cfg_control;
}

int ui_sprite_ui_action_control_anim_set_anim(ui_sprite_ui_action_control_anim_t control_anim, const char * anim) {
    if (control_anim->m_cfg_anim) {
        mem_free(control_anim->m_module->m_alloc, control_anim->m_cfg_anim);
    }

    if (anim) {
        control_anim->m_cfg_anim = cpe_str_mem_dup_trim(control_anim->m_module->m_alloc, anim);
        if (control_anim->m_cfg_anim == NULL) {
            CPE_ERROR(control_anim->m_module->m_em, "ui_sprite_ui_action_control_anim_set_anim: dup str fail!");
            return -1;
        }
    }
    else {
        control_anim->m_cfg_anim = NULL;
    }

    return 0;
}

const char * ui_sprite_ui_action_control_anim_anim(ui_sprite_ui_action_control_anim_t control_anim) {
    return control_anim->m_cfg_anim;
}

int ui_sprite_ui_action_control_anim_set_init(ui_sprite_ui_action_control_anim_t control_anim, const char * init) {
    if (control_anim->m_cfg_init) {
        mem_free(control_anim->m_module->m_alloc, control_anim->m_cfg_init);
    }

    if (init) {
        control_anim->m_cfg_init = cpe_str_mem_dup_trim(control_anim->m_module->m_alloc, init);
        if (control_anim->m_cfg_init == NULL) {
            CPE_ERROR(control_anim->m_module->m_em, "ui_sprite_ui_action_control_init_set_init: dup str fail!");
            return -1;
        }
    }
    else {
        control_anim->m_cfg_init = NULL;
    }

    return 0;
}

const char * ui_sprite_ui_action_control_anim_init(ui_sprite_ui_action_control_anim_t control_anim) {
    return control_anim->m_cfg_init;
}

static int ui_sprite_ui_action_control_anim_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_control_anim_t control_anim = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    const char * control_path;
    plugin_ui_control_t control;
    char * str_anim;
    plugin_ui_animation_t animation;
    
    if (control_anim->m_cfg_control == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim-control: enter: control not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (control_anim->m_cfg_anim == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim-control: enter: anim not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    
    control_path = ui_sprite_fsm_action_check_calc_str(gd_app_tmp_buffer(module->m_app), control_anim->m_cfg_control, fsm_action, NULL, module->m_em);
    if (control_path == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim-control: enter: calc control name from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), control_anim->m_cfg_control);
        return -1;
    }

    control = ui_sprite_ui_find_control_from_action(module, fsm_action, control_path);
    if (control == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim-control: control %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), control_path);
        return -1;
    }

    if (control_anim->m_cfg_init) {
        const char * init_attrs = ui_sprite_fsm_action_check_calc_str(
            gd_app_tmp_buffer(module->m_app), control_anim->m_cfg_init, fsm_action, NULL, module->m_em);
        if (plugin_ui_control_bulk_set_attrs(control, init_attrs) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): anim-control: control %s init from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                plugin_ui_control_name(control), init_attrs);
            return -1;
        }
    }
    
    str_anim = cpe_str_mem_dup(module->m_alloc, control_anim->m_cfg_anim);
    if (str_anim == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim-control: anim calc from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), control_anim->m_cfg_anim);
        return -1;
    }
    
    animation = plugin_ui_control_create_animation(control, str_anim);
    if (animation == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim-control: anim %s start fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), str_anim);
        mem_free(module->m_alloc, str_anim);
        return -1;
    }
    mem_free(module->m_alloc, str_anim);

    if (control_anim->m_cfg_delay_ms > 0.0f) {
        plugin_ui_animation_set_delay(animation, control_anim->m_cfg_delay_ms);
    }

    if (control_anim->m_cfg_loop_count != 1) {
        plugin_ui_animation_set_loop(animation, control_anim->m_cfg_loop_count, control_anim->m_cfg_loop_delay_ms);
    }

    if (plugin_ui_animation_start(animation) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): anim-control: anim %s start fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), str_anim);
        plugin_ui_animation_free(animation);
        return -1;
    }
    
    control_anim->m_animation_id = plugin_ui_animation_id(animation);

    if (ui_sprite_fsm_action_life_circle(fsm_action) == ui_sprite_fsm_action_life_circle_working) {
        ui_sprite_fsm_action_start_update(fsm_action);
    }
    
    return 0;
}

static void ui_sprite_ui_action_control_anim_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_control_anim_t control_anim = ui_sprite_fsm_action_data(fsm_action);

    if (control_anim->m_animation_id) {
        plugin_ui_animation_t animation = plugin_ui_animation_find(module->m_env->m_env, control_anim->m_animation_id);
        if (animation == NULL) {
            control_anim->m_animation_id = 0;
        }
    }

    if (control_anim->m_animation_id == 0) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

static void ui_sprite_ui_action_control_anim_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_control_anim_t control_anim = ui_sprite_fsm_action_data(fsm_action);
    
    if (control_anim->m_animation_id) {
        plugin_ui_animation_t animation = plugin_ui_animation_find(module->m_env->m_env, control_anim->m_animation_id);
        if (animation) {
            plugin_ui_animation_free(animation);
        }
        control_anim->m_animation_id = 0;
    }
}

static int ui_sprite_ui_action_control_anim_do_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_action_control_anim_t control_anim = ui_sprite_fsm_action_data(fsm_action);
    control_anim->m_module = ctx;
    control_anim->m_cfg_control = NULL;
    control_anim->m_cfg_anim = NULL;
    control_anim->m_cfg_init = NULL;
    control_anim->m_cfg_delay_ms = 0.0f;
    control_anim->m_cfg_loop_count = 1;
    control_anim->m_cfg_loop_delay_ms = 0.0f;
    control_anim->m_animation_id = 0;
    return 0;
}

static void ui_sprite_ui_action_control_anim_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_control_anim_t control_anim = ui_sprite_fsm_action_data(fsm_action);

    assert(control_anim->m_animation_id == 0);

    if (control_anim->m_cfg_control) {
        mem_free(module->m_alloc, control_anim->m_cfg_control);
        control_anim->m_cfg_control = NULL;
    }

    if (control_anim->m_cfg_anim) {
        mem_free(module->m_alloc, control_anim->m_cfg_anim);
        control_anim->m_cfg_anim = NULL;
    }

    if (control_anim->m_cfg_init) {
        mem_free(module->m_alloc, control_anim->m_cfg_init);
        control_anim->m_cfg_init = NULL;
    }
}

static int ui_sprite_ui_action_control_anim_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
	ui_sprite_ui_action_control_anim_t to_control_anim = ui_sprite_fsm_action_data(to);
	ui_sprite_ui_action_control_anim_t from_control_anim = ui_sprite_fsm_action_data(from);
    
	if (ui_sprite_ui_action_control_anim_do_init(to, ctx)) return -1;

    to_control_anim->m_cfg_delay_ms = from_control_anim->m_cfg_delay_ms;
    to_control_anim->m_cfg_loop_count = from_control_anim->m_cfg_loop_count;
    to_control_anim->m_cfg_loop_delay_ms = from_control_anim->m_cfg_loop_delay_ms;

    if (from_control_anim->m_cfg_control) {
        to_control_anim->m_cfg_control = cpe_str_mem_dup(module->m_alloc, from_control_anim->m_cfg_control);
    }

    if (from_control_anim->m_cfg_init) {
        to_control_anim->m_cfg_init = cpe_str_mem_dup(module->m_alloc, from_control_anim->m_cfg_init);
    }

    if (from_control_anim->m_cfg_anim) {
        to_control_anim->m_cfg_anim = cpe_str_mem_dup(module->m_alloc, from_control_anim->m_cfg_anim);
    }
    
    return 0;
}

int ui_sprite_ui_action_control_anim_regist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_UI_ACTION_CONTROL_ANIM_NAME, sizeof(struct ui_sprite_ui_action_control_anim));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create action %s: meta create fail",
            ui_sprite_ui_module_name(module), UI_SPRITE_UI_ACTION_CONTROL_ANIM_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_ui_action_control_anim_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_ui_action_control_anim_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_ui_action_control_anim_do_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_ui_action_control_anim_copy, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_ui_action_control_anim_update, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_ui_action_control_anim_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_UI_ACTION_CONTROL_ANIM_NAME, ui_sprite_ui_action_control_anim_load, module) != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: create action_control_anim action: add loader %s fail!",
            ui_sprite_ui_module_name(module), UI_SPRITE_UI_ACTION_CONTROL_ANIM_NAME);
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }
    
    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, "ui-move-in-out", ui_sprite_ui_action_control_move_inout_load, module) != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: create action_control_anim action: add loader %s fail!",
            ui_sprite_ui_module_name(module), UI_SPRITE_UI_ACTION_CONTROL_ANIM_NAME);
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_UI_ACTION_CONTROL_ANIM_NAME);
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }
    
    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, "ui-alpha-in-out", ui_sprite_ui_action_control_alpha_inout_load, module) != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: create action_control_anim action: add loader %s fail!",
            ui_sprite_ui_module_name(module), UI_SPRITE_UI_ACTION_CONTROL_ANIM_NAME);
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, "ui-move-in-out");
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_UI_ACTION_CONTROL_ANIM_NAME);
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }
    
    return 0;
}

void ui_sprite_ui_action_control_anim_unregist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_UI_ACTION_CONTROL_ANIM_NAME);
    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, "ui-move-in-out");
    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, "ui-alpha-in-out");
    
    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_UI_ACTION_CONTROL_ANIM_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_ui_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_UI_ACTION_CONTROL_ANIM_NAME = "ui-control-anim";
