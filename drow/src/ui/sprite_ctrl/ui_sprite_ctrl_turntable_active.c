#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_ctrl_turntable_active_i.h"
#include "ui_sprite_ctrl_turntable_member_i.h"
#include "ui_sprite_ctrl_turntable_i.h"

ui_sprite_ctrl_turntable_active_t ui_sprite_ctrl_turntable_active_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CTRL_TURNTABLE_ACTIVE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_ctrl_turntable_active_free(ui_sprite_ctrl_turntable_active_t ctrl) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(ctrl);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_ctrl_turntable_active_set_decorator(ui_sprite_ctrl_turntable_active_t active, const char * decorator) {
    return ui_percent_decorator_setup(&active->m_updator.m_decorator, decorator, active->m_module->m_em);
}

static void ui_sprite_ctrl_turntable_active_on_active(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_ctrl_turntable_active_t active = ctx;
    ui_sprite_ctrl_module_t module = active->m_module;
    ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
    ui_sprite_ctrl_turntable_member_t member = ui_sprite_ctrl_turntable_member_find(entity);
    UI_SPRITE_EVT_CTRL_TURNTABLE_MEMBER_ACTIVE const * evt_data = evt->data;

    if (member == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable-active: on-begin: entity not turntable member!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (member->m_turntable == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable-active: on-begin: entity not join turntable!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    ui_sprite_ctrl_turntable_updator_stop(&active->m_updator, member);

    if (member != member->m_turntable->m_focuse_member) {
        ui_sprite_ctrl_turntable_set_focuse_member(member->m_turntable, member);
    }

    if (member->m_angle == member->m_turntable->m_def.focuse_angle) {
        ui_sprite_ctrl_turntable_update_members_angle(member->m_turntable, member, member->m_angle);
        ui_sprite_ctrl_turntable_update_members_transform(member->m_turntable);
        return;
    }

    ui_sprite_ctrl_turntable_updator_set_max_speed(&active->m_updator, evt_data->max_speed);
    ui_sprite_ctrl_turntable_updator_set_angle(&active->m_updator, member, member->m_turntable->m_def.focuse_angle);

    ui_sprite_fsm_action_sync_update(action, 1);
}

static int ui_sprite_ctrl_turntable_active_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ctrl_module_t module = ctx;
    ui_sprite_ctrl_turntable_active_t active = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);


    if ((ui_sprite_fsm_action_add_event_handler(
             fsm_action, ui_sprite_event_scope_self, 
             "ui_sprite_evt_ctrl_turntable_member_active",
             ui_sprite_ctrl_turntable_active_on_active, active) != 0)
        )
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable-active: enter: add eventer handler fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    return 0;
}

static void ui_sprite_ctrl_turntable_active_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ctrl_turntable_active_t active = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_ctrl_turntable_member_t member = ui_sprite_ctrl_turntable_member_find(entity);

    ui_sprite_ctrl_turntable_updator_stop(&active->m_updator, member);
}

static void ui_sprite_ctrl_turntable_active_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_ctrl_turntable_active_t active = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_ctrl_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_ctrl_turntable_member_t member = ui_sprite_ctrl_turntable_member_find(entity);
    ui_sprite_ctrl_turntable_t turntable;

    if (member == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable-active: on-udpate: entity not turntable member!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (member->m_turntable == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable-active: on-update: entity not join turntable!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    turntable = member->m_turntable;

    if (active->m_updator.m_curent_op_id != turntable->m_curent_op_id) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): turntable-active: on-update: stop for no update lock!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        }
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    ui_sprite_ctrl_turntable_updator_update(&active->m_updator, member, delta);

    ui_sprite_ctrl_turntable_update_members_transform(turntable);

    if (!ui_sprite_ctrl_turntable_updator_is_runing(&active->m_updator)) {
        if (member != turntable->m_focuse_member) {
            ui_sprite_ctrl_turntable_set_focuse_member(turntable, member);
        }

        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

static int ui_sprite_ctrl_turntable_active_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ctrl_turntable_active_t ctrl = ui_sprite_fsm_action_data(fsm_action);

    bzero(ctrl, sizeof(*ctrl));

    ctrl->m_module = ctx;

    return 0;
}

static void ui_sprite_ctrl_turntable_active_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ctrl_turntable_active_t active = ui_sprite_fsm_action_data(fsm_action);

    assert(active->m_updator.m_curent_op_id == 0);
}

static int ui_sprite_ctrl_turntable_active_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_ctrl_turntable_active_t to_active = ui_sprite_fsm_action_data(to);
    ui_sprite_ctrl_turntable_active_t from_active = ui_sprite_fsm_action_data(from);

    if (ui_sprite_ctrl_turntable_active_init(to, ctx)) return -1;

    to_active->m_decorator = from_active->m_decorator;

    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_ctrl_turntable_active_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_ctrl_module_t module = ctx;
    ui_sprite_ctrl_turntable_active_t turntable_active = ui_sprite_ctrl_turntable_active_create(fsm_state, name);
    const char * decorator;

    if (turntable_active == NULL) {
        CPE_ERROR(module->m_em, "%s: create ctrl_turntable_active action: create fail!", ui_sprite_ctrl_module_name(module));
        return NULL;
    }

    if ((decorator = cfg_get_string(cfg, "decorator", NULL))) {
        if (ui_sprite_ctrl_turntable_active_set_decorator(turntable_active, decorator) != 0) {
            CPE_ERROR(module->m_em, "%s: create ctrl_turntable_active action: create fail!", ui_sprite_ctrl_module_name(module));
            ui_sprite_ctrl_turntable_active_free(turntable_active);
            return NULL;
        }
    }

    return ui_sprite_fsm_action_from_data(turntable_active);
}

int ui_sprite_ctrl_turntable_active_regist(ui_sprite_ctrl_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_CTRL_TURNTABLE_ACTIVE_NAME, sizeof(struct ui_sprite_ctrl_turntable_active));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ctrl camera ctrl register: meta create fail",
            ui_sprite_ctrl_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_ctrl_turntable_active_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_ctrl_turntable_active_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_ctrl_turntable_active_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_ctrl_turntable_active_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_ctrl_turntable_active_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_ctrl_turntable_active_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_CTRL_TURNTABLE_ACTIVE_NAME, ui_sprite_ctrl_turntable_active_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_ctrl_turntable_active_unregist(ui_sprite_ctrl_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CTRL_TURNTABLE_ACTIVE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ctrl camera ctrl unregister: meta not exist",
            ui_sprite_ctrl_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CTRL_TURNTABLE_ACTIVE_NAME);
    }
}

const char * UI_SPRITE_CTRL_TURNTABLE_ACTIVE_NAME = "ctrl-turntable-active";

