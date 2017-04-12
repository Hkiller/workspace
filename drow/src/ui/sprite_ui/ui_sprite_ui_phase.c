#include <assert.h>
#include "cpe/dr/dr_metalib_manage.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_repository.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_fsm/ui_sprite_fsm_component.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_fsm.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_basic/ui_sprite_basic_noop.h"
#include "ui_sprite_ui_phase_i.h"

int ui_sprite_ui_env_phase_init(void * ctx, plugin_ui_env_t b_env, plugin_ui_phase_t b_phase) {
    ui_sprite_ui_phase_t phase = plugin_ui_phase_data(b_phase);
    phase->m_fsm = NULL;
    phase->m_phase_state = NULL;
    return 0;
}

void ui_sprite_ui_env_phase_fini(void * ctx, plugin_ui_env_t b_env, plugin_ui_phase_t b_phase) {
    ui_sprite_ui_phase_t phase = plugin_ui_phase_data(b_phase);

    assert(phase->m_phase_state == NULL);
    
    if (phase->m_fsm) {
        ui_sprite_entity_t proto_entity = ui_sprite_fsm_to_entity(phase->m_fsm);
        ui_sprite_entity_free(proto_entity);
        phase->m_fsm = NULL;
    }
}

int ui_sprite_ui_env_phase_enter(void * ctx, plugin_ui_env_t b_env, plugin_ui_phase_t b_phase) {
    ui_sprite_ui_env_t env = ctx;
    ui_sprite_ui_phase_t phase = plugin_ui_phase_data(b_phase);
    ui_sprite_component_t component;
    ui_sprite_fsm_ins_t fsm;
    ui_sprite_fsm_state_t fsm_state;
    ui_sprite_fsm_ins_t phase_fsm;
    ui_sprite_fsm_action_t fsm_action;
    ui_sprite_fsm_state_t phase_stae;
    ui_sprite_basic_noop_t keep_state_action;

    assert(phase->m_phase_state == NULL);
    
    if (phase->m_fsm == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_phase_enter: %s: no fsm loaded!",
            plugin_ui_phase_name(b_phase));
        return -1;
    }

    component = ui_sprite_component_find(env->m_entity, UI_SPRITE_FSM_COMPONENT_FSM_NAME);
    assert(component);

    fsm = ui_sprite_component_data(component);

    fsm_state = ui_sprite_fsm_current_state(fsm);
    assert(fsm_state);

    phase_fsm = (ui_sprite_fsm_ins_t)ui_sprite_fsm_action_fsm_create(fsm_state, "Phase");
    if (phase_fsm == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_phase_enter: %s: crate node fsm fail!",
            plugin_ui_phase_name(b_phase));
        return -1;
    }

    fsm_action = ui_sprite_fsm_action_from_data(phase_fsm);

    phase_stae = ui_sprite_fsm_state_create(phase_fsm, plugin_ui_phase_name(b_phase));
    if (phase_stae == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_phase_enter: %s: create phase state fail!",
            plugin_ui_phase_name(b_phase));
        ui_sprite_fsm_action_free(fsm_action);
        return -1;
    }

    if (ui_sprite_fsm_set_default_state(phase_fsm, plugin_ui_phase_name(b_phase)) != 0) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_phae_enter: %s: set default state fail!",
            plugin_ui_phase_name(b_phase));
        ui_sprite_fsm_action_free(fsm_action);
        return -1;
    }

    keep_state_action = ui_sprite_basic_noop_create(phase_stae, "keep-state");
    if (keep_state_action == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_phase_enter: %s: create keep state action fail!",
            plugin_ui_phase_name(b_phase));
        ui_sprite_fsm_action_free(fsm_action);
        return -1;
    }

    if (ui_sprite_fsm_action_set_life_circle(
            ui_sprite_fsm_action_from_data(keep_state_action), ui_sprite_fsm_action_life_circle_endless)
        != 0)
    {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_phase_enter: %s: set keep state action life_circle fail!",
            plugin_ui_phase_name(b_phase));
        ui_sprite_fsm_action_free(fsm_action);
        return -1;
    }

    if (ui_sprite_fsm_action_enter(fsm_action) != 0) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_phase_enter: %s: phase action enter fail!",
            plugin_ui_phase_name(b_phase));
        ui_sprite_fsm_action_free(fsm_action);
        return -1;
    }
    
    phase->m_phase_state = phase_stae;
    
    return 0;
}

void ui_sprite_ui_env_phase_leave(void * ctx, plugin_ui_env_t b_env, plugin_ui_phase_t b_phase) {
    ui_sprite_ui_phase_t phase = plugin_ui_phase_data(b_phase);
    ui_sprite_fsm_ins_t phase_fsm;
    ui_sprite_fsm_action_t phase_action;
    
    assert(phase->m_phase_state);

    phase_fsm = ui_sprite_fsm_state_fsm(phase->m_phase_state);
    assert(phase_fsm);
    
    phase_action = ui_sprite_fsm_action_from_data(phase_fsm);
    assert(phase_action);

    ui_sprite_fsm_action_free(phase_action);

    phase->m_phase_state = NULL;
}
