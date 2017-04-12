#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_repository.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_fsm/ui_sprite_fsm_component.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_fsm.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "plugin/ui/plugin_ui_state_node.h"
#include "ui/sprite_ui/ui_sprite_ui_action_navigation.h"
#include "ui_sprite_ui_state_i.h"
#include "ui_sprite_ui_navigation_i.h"

int ui_sprite_ui_env_state_init(void * ctx, plugin_ui_state_t b_state) {
    ui_sprite_ui_state_t state = plugin_ui_state_data(b_state);

    state->m_env = ctx;
    state->m_fsm_state = NULL;
    
    return 0;
}

void ui_sprite_ui_env_state_fini(void * ctx, plugin_ui_state_t b_state) {
    ui_sprite_ui_state_t state = plugin_ui_state_data(b_state);
    state->m_fsm_state = NULL;
}

int ui_sprite_ui_env_state_node_active(
    void * ctx, plugin_ui_state_node_t state_node, plugin_ui_state_t b_state)
{
    ui_sprite_ui_env_t env = ctx;
    ui_sprite_fsm_component_fsm_t fsm;
    ui_sprite_fsm_action_t fsm_action;
    plugin_ui_phase_t b_phase = plugin_ui_state_phase(b_state);
    ui_sprite_ui_state_t state = plugin_ui_state_data(b_state);
    ui_sprite_ui_phase_t phase;
    ui_sprite_fsm_ins_t node_fsm;
    ui_sprite_fsm_state_t node_state;
    char fsm_action_name[64];
    struct ui_sprite_fsm_action_it from_action_it;
    ui_sprite_fsm_action_t from_action;
    struct plugin_ui_navigation_it navigation_it;
    plugin_ui_navigation_t b_navigation;
    dr_data_t state_node_data = plugin_ui_state_node_data(state_node);

    fsm = ui_sprite_fsm_component_find(env->m_entity);
    if (fsm == NULL) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_state_node_active: no current fsm!");
        return -1;
    }

    phase = plugin_ui_phase_data(b_phase);
    if (phase->m_phase_state == NULL) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_state_node_active: phase no state!");
        return 0;
    }

    snprintf(fsm_action_name, sizeof(fsm_action_name), "L%d", plugin_ui_state_node_level(state_node));
    node_fsm = (ui_sprite_fsm_ins_t)ui_sprite_fsm_action_fsm_create(phase->m_phase_state, fsm_action_name);
    if (node_fsm == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_state_node_active: state %s crate node fsm %s fail!",
            plugin_ui_phase_name(b_phase), fsm_action_name);
        return -1;
    }

    fsm_action = ui_sprite_fsm_action_from_data(node_fsm);
    
    node_state = ui_sprite_fsm_state_create(node_fsm, plugin_ui_state_name(b_state));
    if (node_state == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_state_node_active: state %s action %s create node state fail!",
            plugin_ui_phase_name(b_phase), fsm_action_name);
        ui_sprite_fsm_action_free(fsm_action);
        return -1;
    }

    if (state_node_data) {
        struct ui_sprite_event enter_evt;

        enter_evt.from_entity_id = ui_sprite_entity_id(env->m_entity);
        enter_evt.meta = state_node_data->m_meta;
        enter_evt.data = state_node_data->m_data;
        enter_evt.size = state_node_data->m_size;

        if (ui_sprite_fsm_state_set_enter_event(node_state, &enter_evt) != 0) {
            CPE_ERROR(
                env->m_module->m_em, "ui_sprite_ui_env_state_node_active: state %s action %s set state enter event fail!",
                plugin_ui_phase_name(b_phase), fsm_action_name);
            ui_sprite_fsm_action_free(fsm_action);
            return -1;
        }
    }

    ui_sprite_fsm_state_actions(&from_action_it, state->m_fsm_state);
    while((from_action = ui_sprite_fsm_state_it_next(&from_action_it))) {
        if (ui_sprite_fsm_action_clone(node_state, from_action) == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "ui_sprite_ui_env_state_node_active: state %s action %s: copay state actions fail!",
                plugin_ui_phase_name(b_phase), fsm_action_name);
            ui_sprite_fsm_action_free(fsm_action);
            return -1;
        }
    }

    /*跳转的处理 */
    plugin_ui_state_navigations_to(b_state, &navigation_it);
    while((b_navigation = plugin_ui_navigation_it_next(&navigation_it))) {
        ui_sprite_ui_navigation_t navigation = plugin_ui_navigation_data(b_navigation);
        ui_sprite_ui_action_navigation_t action_navigation;

        if (navigation->m_event == NULL) continue;

        action_navigation = ui_sprite_ui_action_navigation_create(node_state, "");
        if (action_navigation == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "ui_sprite_ui_env_state_node_active: state %s action %s: create navigation state fail!",
                plugin_ui_phase_name(b_phase), fsm_action_name);
            ui_sprite_fsm_action_free(fsm_action);
            return -1;
        }

        if (ui_sprite_ui_action_navigation_set_event(
            action_navigation, navigation->m_event, plugin_ui_navigation_condition(b_navigation)) != 0)
        {
            CPE_ERROR(
                env->m_module->m_em, "ui_sprite_ui_env_state_node_active: state %s action %s: set event state fail!",
                plugin_ui_phase_name(b_phase), fsm_action_name);
            ui_sprite_fsm_action_free(fsm_action);
            return -1;
        }

        ui_sprite_ui_action_navigation_set_navigation(action_navigation, b_navigation);

        if (ui_sprite_fsm_action_set_life_circle(ui_sprite_fsm_action_from_data(action_navigation), ui_sprite_fsm_action_life_circle_passive) != 0) {
            CPE_ERROR(
                env->m_module->m_em, "ui_sprite_ui_env_state_node_active: state %s action %s: set event life circle fail!",
                plugin_ui_phase_name(b_phase), fsm_action_name);
            ui_sprite_fsm_action_free(fsm_action);
            return -1;
        }
    }

    if (ui_sprite_fsm_set_default_state(node_fsm, plugin_ui_state_name(b_state)) != 0) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_state_node_active: set default state Do fail!");
        ui_sprite_fsm_action_free(fsm_action);
        return -1;
    }

    if (ui_sprite_fsm_action_enter(fsm_action) != 0) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_state_node_active: enter action Do fail!");
        ui_sprite_fsm_action_free(fsm_action);
        return -1;
    }

    return 0;
}

uint8_t ui_sprite_ui_env_state_node_is_active(void * ctx, plugin_ui_state_node_t state_node) {
    ui_sprite_ui_env_t env = ctx;
    plugin_ui_state_t b_state = plugin_ui_state_node_current_state(state_node);
    plugin_ui_phase_t b_phase = b_state ? plugin_ui_state_phase(b_state) : NULL;
    ui_sprite_ui_phase_t phase;
    ui_sprite_fsm_action_t fsm_action;
    char fsm_action_name[64];

    if (b_phase == NULL) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_state_node_is_active: state node non current state!");
        return 0;
    }

    phase = plugin_ui_phase_data(b_phase);
    if (phase->m_phase_state == NULL) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_state_node_is_active: phase no state!");
        return 0;
    }
    
    snprintf(fsm_action_name, sizeof(fsm_action_name), "L%d", plugin_ui_state_node_level(state_node));
    fsm_action = ui_sprite_fsm_action_find_by_name(phase->m_phase_state, fsm_action_name);
    if (fsm_action == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_env_state_node_is_active: state %s action %s not exist!",
            ui_sprite_fsm_state_name(phase->m_phase_state), fsm_action_name);
        return 0;
    }

    switch(ui_sprite_fsm_action_runing_state(fsm_action)) {
    case ui_sprite_fsm_action_state_waiting:
        return 1;
    case ui_sprite_fsm_action_state_runing: {
        ui_sprite_fsm_state_t cur_state = ui_sprite_fsm_current_state((ui_sprite_fsm_ins_t)ui_sprite_fsm_action_data(fsm_action));
        uint16_t actoun_count = cur_state ? ui_sprite_fsm_state_count_left_actions(cur_state) : 0;
        return actoun_count <= 0 ? 0 : 1;
    }
    default:
        return 0;
    }
}

void ui_sprite_ui_env_state_node_deactive(void * ctx, plugin_ui_state_node_t state_node) {
    ui_sprite_ui_env_t env = ctx;
    plugin_ui_state_t b_state = plugin_ui_state_node_current_state(state_node);
    plugin_ui_phase_t b_phase = b_state ? plugin_ui_state_phase(b_state) : NULL;
    ui_sprite_ui_phase_t phase;
    ui_sprite_fsm_action_t fsm_action;
    char fsm_action_name[64];

    if (b_phase == NULL) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_state_node_deactive: state node non current state!");
        return;
    }

    phase = plugin_ui_phase_data(b_phase);
    if (phase->m_phase_state == NULL) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_state_node_deactive: phase no state!");
        return;
    }
    
    snprintf(fsm_action_name, sizeof(fsm_action_name), "L%d", plugin_ui_state_node_level(state_node));
    fsm_action = ui_sprite_fsm_action_find_by_name(phase->m_phase_state, fsm_action_name);
    if (fsm_action == NULL) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_ui_env_state_node_deactive: action %s not exist!", fsm_action_name)
        return;
    }

    ui_sprite_fsm_action_free(fsm_action);
}
