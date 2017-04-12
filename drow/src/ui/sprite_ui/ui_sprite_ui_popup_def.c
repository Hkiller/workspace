#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "plugin/ui/plugin_ui_popup.h"
#include "plugin/ui/plugin_ui_popup_def.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_repository.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_fsm/ui_sprite_fsm_component.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_basic/ui_sprite_basic_noop.h"
#include "ui_sprite_ui_popup_def_i.h"
#include "ui_sprite_ui_env_i.h"
#include "ui_sprite_ui_action_guard_popup_i.h"

int ui_sprite_ui_env_popup_def_init(void * ctx, plugin_ui_env_t b_env, plugin_ui_popup_def_t b_popup_def) {
    ui_sprite_ui_popup_def_t popup_def = plugin_ui_popup_def_data(b_popup_def);
    popup_def->m_fsm = NULL;
    return 0;
}

void ui_sprite_ui_env_popup_def_fini(void * ctx, plugin_ui_env_t b_env, plugin_ui_popup_def_t b_popup_def) {
    ui_sprite_ui_popup_def_t popup_def = plugin_ui_popup_def_data(b_popup_def);
    
    if (popup_def->m_fsm) {
        ui_sprite_entity_t proto_entity = ui_sprite_fsm_to_entity(popup_def->m_fsm);
        ui_sprite_entity_free(proto_entity);
        popup_def->m_fsm = NULL;
    }
}

int ui_sprite_ui_env_popup_enter(void * ctx, plugin_ui_env_t b_env, plugin_ui_popup_t b_popup) {
    ui_sprite_ui_env_t env = ctx;
    plugin_ui_popup_def_t b_popup_def = plugin_ui_popup_def(b_popup);
    ui_sprite_ui_popup_def_t popup_def;
    ui_sprite_component_t component;
    ui_sprite_fsm_ins_t fsm;
    ui_sprite_fsm_state_t fsm_state;
    ui_sprite_fsm_ins_t popup_fsm;
    ui_sprite_fsm_action_t popup_fsm_action;
    ui_sprite_fsm_state_t popup_state;
    ui_sprite_ui_action_guard_popup_t popup_guard;
    char fsm_action_name[64];
    ui_sprite_fsm_ins_t work_fsm;

    if (b_popup_def == NULL) return 0;

    popup_def = plugin_ui_popup_def_data(b_popup_def);
    if (popup_def->m_fsm == NULL) return 0;

    component = ui_sprite_component_find(env->m_entity, UI_SPRITE_FSM_COMPONENT_FSM_NAME);
    assert(component);

    fsm = ui_sprite_component_data(component);

    fsm_state = ui_sprite_fsm_current_state(fsm);
    assert(fsm_state);

    snprintf(fsm_action_name, sizeof(fsm_action_name), "P%d", plugin_ui_popup_id(b_popup));

    popup_fsm = (ui_sprite_fsm_ins_t)ui_sprite_fsm_action_fsm_create(fsm_state, fsm_action_name);
    if (popup_fsm == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_popup_enter: %d(%s): crate popup fsm fail!",
            plugin_ui_popup_id(b_popup), plugin_ui_popup_name(b_popup));
        return -1;
    }

    popup_fsm_action = ui_sprite_fsm_action_from_data(popup_fsm);

    popup_state = ui_sprite_fsm_state_create(popup_fsm, plugin_ui_popup_name(b_popup));
    if (popup_state == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_popup_enter: %d(%s): create popup state fail!",
            plugin_ui_popup_id(b_popup), plugin_ui_popup_name(b_popup));
        ui_sprite_fsm_action_free(popup_fsm_action);
        return -1;
    }

    if (ui_sprite_fsm_set_default_state(popup_fsm, plugin_ui_popup_name(b_popup)) != 0) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_popup_enter: (%d)%s: set default state fail!",
            plugin_ui_popup_id(b_popup), plugin_ui_popup_name(b_popup));
        ui_sprite_fsm_action_free(popup_fsm_action);
        return -1;
    }

    popup_guard = ui_sprite_ui_action_guard_popup_create(popup_state, "Guard");
    if (popup_guard == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_popup_enter: (%d)%s: create popup guard action fail!",
            plugin_ui_popup_id(b_popup), plugin_ui_popup_name(b_popup));
        ui_sprite_fsm_action_free(popup_fsm_action);
        return -1;
    }
    ui_sprite_fsm_action_set_life_circle(ui_sprite_fsm_action_from_data(popup_guard), ui_sprite_fsm_action_life_circle_passive);
    
    work_fsm = (ui_sprite_fsm_ins_t)ui_sprite_fsm_action_fsm_create(popup_state, "Do");
    if (work_fsm == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_popup_enter: %d(%s): crate work fsm fail!",
            plugin_ui_popup_id(b_popup), plugin_ui_popup_name(b_popup));
        ui_sprite_fsm_action_free(popup_fsm_action);
        return -1;
    }
    
    if (ui_sprite_fsm_ins_copy(work_fsm, popup_def->m_fsm) != 0) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_popup_enter: %d(%s): copy work fsm fail!",
            plugin_ui_popup_id(b_popup), plugin_ui_popup_name(b_popup));
        ui_sprite_fsm_action_free(popup_fsm_action);
        return -1;
    }

    if (ui_sprite_fsm_action_enter(popup_fsm_action) != 0) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_popup_enter: %d(%s): action enter fail!",
            plugin_ui_popup_id(b_popup), plugin_ui_popup_name(b_popup));
        ui_sprite_fsm_action_free(popup_fsm_action);
        return -1;
    }
    
    return 0;
}

void ui_sprite_ui_env_popup_leave(void * ctx, plugin_ui_env_t b_env, plugin_ui_popup_t b_popup) {
    ui_sprite_ui_env_t env = ctx;    
    plugin_ui_popup_def_t b_popup_def = plugin_ui_popup_def(b_popup);
    ui_sprite_ui_popup_def_t popup_def;
    ui_sprite_component_t component;
    ui_sprite_fsm_ins_t fsm;
    ui_sprite_fsm_state_t fsm_state;
    ui_sprite_fsm_action_t fsm_action;
    char fsm_action_name[64];
    ui_sprite_fsm_ins_t popup_fsm;
    ui_sprite_fsm_state_t popup_state;
    ui_sprite_fsm_action_t popup_guard_action;
    ui_sprite_ui_action_guard_popup_t popup_guard;

    if (b_popup_def == NULL) return;

    popup_def = plugin_ui_popup_def_data(b_popup_def);
    if (popup_def->m_fsm == NULL) return;

    component = ui_sprite_component_find(env->m_entity, UI_SPRITE_FSM_COMPONENT_FSM_NAME);
    assert(component);

    fsm = ui_sprite_component_data(component);

    fsm_state = ui_sprite_fsm_current_state(fsm);
    assert(fsm_state);

    snprintf(fsm_action_name, sizeof(fsm_action_name), "P%d", plugin_ui_popup_id(b_popup));
    fsm_action = ui_sprite_fsm_action_find_by_name(fsm_state, fsm_action_name);
    if (fsm_action == NULL) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_popup_leave: action %s not exist!", fsm_action_name)
        return;
    }

    popup_fsm = (ui_sprite_fsm_ins_t)ui_sprite_fsm_action_data(fsm_action);
    popup_state = ui_sprite_fsm_current_state(popup_fsm);
    
    popup_guard_action = ui_sprite_fsm_action_find_by_name(popup_state, "Guard");
    if (popup_guard_action == NULL) {
        assert(0);
        CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_popup_leave: popup guard action not exist!");
        return;
    }

    popup_guard = ui_sprite_fsm_action_data(popup_guard_action);
    if (!popup_guard->m_free_from_guard) {
        ui_sprite_fsm_action_free(fsm_action);
    }
}    

int ui_sprite_ui_env_popup_def_load(ui_sprite_ui_env_t env, plugin_ui_popup_def_t b_popup_def, ui_sprite_cfg_loader_t loader, cfg_t cfg) {
    ui_sprite_ui_popup_def_t popup_def = plugin_ui_popup_def_data(b_popup_def);
    ui_sprite_world_t world = ui_sprite_entity_world(env->m_entity);
    char fsm_proto_name[64];
    
    assert(popup_def->m_fsm == NULL);

    snprintf(fsm_proto_name, sizeof(fsm_proto_name), "Popup-%s", plugin_ui_popup_def_name(b_popup_def));
    
    popup_def->m_fsm =
        ui_sprite_cfg_loader_load_fsm_proto_from_cfg(loader, world, fsm_proto_name, cfg);
    if (popup_def->m_fsm == NULL) {
        return -1;
    }

    return 0;
}
