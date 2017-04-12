#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_ctrl_turntable_join_i.h"
#include "ui_sprite_ctrl_turntable_member_i.h"
#include "ui_sprite_ctrl_turntable_i.h"

ui_sprite_ctrl_turntable_join_t ui_sprite_ctrl_turntable_join_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CTRL_TURNTABLE_JOIN_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_ctrl_turntable_join_free(ui_sprite_ctrl_turntable_join_t ctrl) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(ctrl);
    ui_sprite_fsm_action_free(fsm_action);
}

const char * ui_sprite_ctrl_turntable_join_turntable(ui_sprite_ctrl_turntable_join_t ctrl) {
    return ctrl->m_turntable_name;
}

void ui_sprite_ctrl_turntable_join_set_turntable(ui_sprite_ctrl_turntable_join_t ctrl, const char * turntable) {
    cpe_str_dup(ctrl->m_turntable_name, sizeof(ctrl->m_turntable_name), turntable);
}

static int ui_sprite_ctrl_turntable_join_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ctrl_module_t module = ctx;
    ui_sprite_ctrl_turntable_join_t ctrl = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_ctrl_turntable_member_t member = ui_sprite_ctrl_turntable_member_find(entity);
	ui_sprite_entity_t turntable_entity;
    ui_sprite_ctrl_turntable_t turntable;

    if (member == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable join: enter: entity is not turntable member!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    turntable_entity = ui_sprite_entity_find_by_name(ui_sprite_entity_world(entity), ctrl->m_turntable_name);
    if (turntable_entity == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable join: enter: turntable entity %s is not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ctrl->m_turntable_name);
        return -1;
    }

    turntable = ui_sprite_ctrl_turntable_find(turntable_entity);
    if (turntable == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable join: enter: turntable entity %d(%s) is not turntable!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_entity_id(turntable_entity), ui_sprite_entity_name(turntable_entity));
        return -1;
    }

    if (ui_sprite_ctrl_turntable_add_member(turntable, member) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable join: enter: join to turntable entity %d(%s) fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_entity_id(turntable_entity), ui_sprite_entity_name(turntable_entity));
        return -1;
    }

    if (turntable->m_focuse_member) {
        ui_sprite_ctrl_turntable_update_members_angle(turntable, turntable->m_focuse_member, turntable->m_def.focuse_angle);
        ui_sprite_ctrl_turntable_update_members_transform(turntable);
    }

    if (ui_sprite_fsm_action_life_circle(fsm_action) == ui_sprite_fsm_action_life_circle_working) {
        ui_sprite_fsm_action_start_update(fsm_action);
    }

    return 0;
}

static void ui_sprite_ctrl_turntable_join_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ctrl_module_t module = ctx;
    ui_sprite_ctrl_turntable_join_t ctrl = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_ctrl_turntable_member_t member = ui_sprite_ctrl_turntable_member_find(entity);
	ui_sprite_entity_t turntable_entity;

    if (member == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable join: exit: entity is not turntable member!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (member->m_turntable == NULL) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): turntable exit: exit: member is not join, skip!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        }
        return;
    }

    turntable_entity = ui_sprite_component_entity(ui_sprite_component_from_data(member->m_turntable));
    if (strcmp(ui_sprite_entity_name(turntable_entity), ctrl->m_turntable_name) != 0) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): turntable join: exit: member is join %s, not %s, skip!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                ui_sprite_entity_name(turntable_entity), ctrl->m_turntable_name);
        }
        return;
    }

    ui_sprite_ctrl_turntable_remove_member(member->m_turntable, member);
}

static void ui_sprite_ctrl_turntable_join_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_ctrl_module_t module = ctx;
    ui_sprite_ctrl_turntable_join_t ctrl = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_ctrl_turntable_member_t member = ui_sprite_ctrl_turntable_member_find(entity);
	ui_sprite_entity_t turntable_entity;

    if (member == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable join: update: entity is not turntable member!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    if (member->m_turntable == NULL) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): turntable join: update: member is not join, stop!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        }
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    turntable_entity = ui_sprite_component_entity(ui_sprite_component_from_data(member->m_turntable));
    if (strcmp(ui_sprite_entity_name(turntable_entity), ctrl->m_turntable_name) != 0) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): turntable join: update: member is join %s, not %s, stop!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                ui_sprite_entity_name(turntable_entity), ctrl->m_turntable_name);
        }
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }
}

static int ui_sprite_ctrl_turntable_join_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ctrl_turntable_join_t ctrl = ui_sprite_fsm_action_data(fsm_action);

    bzero(ctrl, sizeof(*ctrl));

    ctrl->m_module = ctx;

    return 0;
}

static void ui_sprite_ctrl_turntable_join_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_ctrl_turntable_join_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_ctrl_turntable_join_t to_ctrl = ui_sprite_fsm_action_data(to);
    ui_sprite_ctrl_turntable_join_t from_ctrl = ui_sprite_fsm_action_data(from);

    if (ui_sprite_ctrl_turntable_join_init(to, ctx)) return -1;

    cpe_str_dup(to_ctrl->m_turntable_name, sizeof(to_ctrl->m_turntable_name), from_ctrl->m_turntable_name);

    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_ctrl_turntable_join_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_ctrl_module_t module = ctx;
    ui_sprite_ctrl_turntable_join_t ctrl = ui_sprite_ctrl_turntable_join_create(fsm_state, name);
    const char * turntable;

    turntable = cfg_get_string(cfg, "turntable", NULL);
    if (turntable == NULL) {
        CPE_ERROR(module->m_em, "%s: create ctrl_turntable_join action: turntable not configured!", ui_sprite_ctrl_module_name(module));
        return NULL;
    }

    if (ctrl == NULL) {
        CPE_ERROR(module->m_em, "%s: create ctrl_turntable_join action: create fail!", ui_sprite_ctrl_module_name(module));
        return NULL;
    }

    ui_sprite_ctrl_turntable_join_set_turntable(ctrl, turntable);

    return ui_sprite_fsm_action_from_data(ctrl);
}

int ui_sprite_ctrl_turntable_join_regist(ui_sprite_ctrl_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_CTRL_TURNTABLE_JOIN_NAME, sizeof(struct ui_sprite_ctrl_turntable_join));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ctrl camera ctrl register: meta create fail",
            ui_sprite_ctrl_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_ctrl_turntable_join_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_ctrl_turntable_join_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_ctrl_turntable_join_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_ctrl_turntable_join_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_ctrl_turntable_join_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_ctrl_turntable_join_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_CTRL_TURNTABLE_JOIN_NAME, ui_sprite_ctrl_turntable_join_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_ctrl_turntable_join_unregist(ui_sprite_ctrl_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CTRL_TURNTABLE_JOIN_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ctrl camera ctrl unregister: meta not exist",
            ui_sprite_ctrl_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CTRL_TURNTABLE_JOIN_NAME);
    }
}

const char * UI_SPRITE_CTRL_TURNTABLE_JOIN_NAME = "ctrl-turntable-join";

