#include <assert.h>
#include "plugin/ui/plugin_ui_control.h"
#include "plugin/ui/plugin_ui_phase.h"
#include "plugin/ui/plugin_ui_state.h"
#include "plugin/ui/plugin_ui_navigation.h"
#include "plugin/ui/plugin_ui_search.h"
#include "plugin/ui/plugin_ui_state_node.h"
#include "plugin/ui/plugin_ui_phase_node.h"
#include "plugin_ui_exec_step_to_state_i.h"

static plugin_ui_exec_step_result_t
plugin_ui_exec_step_to_state_exec(plugin_ui_exec_step_t step, plugin_ui_exec_action_t o_action);

plugin_ui_exec_step_to_state_t
plugin_ui_exec_step_to_state_create(plugin_ui_exec_plan_t plan, plugin_ui_state_t target_state) {
    plugin_ui_module_t module = plan->m_env->m_module;
    plugin_ui_exec_step_t step;
    plugin_ui_exec_step_to_state_t to_state;

    step = plugin_ui_exec_step_create(plan, sizeof(struct plugin_ui_exec_step_to_state), plugin_ui_exec_step_to_state_exec);
    if (step == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_exec_step_to_state_create: create step fail!");
        return NULL;
    }

    to_state = (plugin_ui_exec_step_to_state_t)plugin_ui_exec_step_data(step);
    to_state->m_target_state = target_state;
    to_state->m_cur_phase = NULL;
    to_state->m_next_state = NULL;

    return to_state;
}

plugin_ui_exec_step_result_t
plugin_ui_exec_step_to_state_exec(plugin_ui_exec_step_t step, plugin_ui_exec_action_t o_action) {
    plugin_ui_module_t module = step->m_plan->m_env->m_module;
    plugin_ui_exec_step_to_state_t to_state;
    plugin_ui_state_node_t cur_state_node;
    plugin_ui_state_t cur_state;
    const char * next_control_name;
    
    to_state = (plugin_ui_exec_step_to_state_t)plugin_ui_exec_step_data(step);

    if (to_state->m_cur_phase == NULL) {
        to_state->m_cur_phase = plugin_ui_phase_node_current(step->m_plan->m_env);
        if (to_state->m_cur_phase == NULL) {
            CPE_ERROR(module->m_em, "plugin_ui_exec_step_to_state_exec: no curent phase");
            return plugin_ui_exec_step_fail;
        }
    }
    else {
        plugin_ui_phase_node_t now_phase = plugin_ui_phase_node_current(step->m_plan->m_env);
        if (now_phase != to_state->m_cur_phase) {
            CPE_ERROR(module->m_em, "plugin_ui_exec_step_to_state_exec: curent phase mismatch");
            return plugin_ui_exec_step_fail;
        }
    }
    
    cur_state_node = plugin_ui_state_node_current(to_state->m_cur_phase);
    if (cur_state_node == NULL) {
        o_action->m_type = plugin_ui_exec_action_noop;
        return plugin_ui_exec_step_success;
    }
    cur_state = plugin_ui_state_node_process_state(cur_state_node);
    
    if (cur_state == to_state->m_target_state) {
        return plugin_ui_exec_step_done;
    }

    if (to_state->m_last_search_state != cur_state) {
        plugin_ui_navigation_t navigation;

        navigation = plugin_ui_search_next_navigation_to_state(to_state->m_cur_phase, plugin_ui_state_name(to_state->m_target_state));
        if (navigation == NULL) {
            CPE_ERROR(
                module->m_em, "plugin_ui_exec_step_to_state_exec: phase %s search path from state %s to %s fail!",
                plugin_ui_phase_name(plugin_ui_phase_node_process_phase(to_state->m_cur_phase)),
                plugin_ui_state_node_name(cur_state_node),
                plugin_ui_state_name(to_state->m_target_state));
            return plugin_ui_exec_step_fail;
        }

        if (plugin_ui_navigation_trigger_control(navigation) == NULL) {
            CPE_ERROR(
                module->m_em, "plugin_ui_exec_step_to_state_exec: phase %s search path from state %s to %s, navigation no trigger control!",
                plugin_ui_phase_name(plugin_ui_phase_node_process_phase(to_state->m_cur_phase)),
                plugin_ui_state_node_name(cur_state_node),
                plugin_ui_state_name(to_state->m_target_state));
            return plugin_ui_exec_step_fail;
        }

        to_state->m_last_search_state = cur_state;
        to_state->m_last_navigation = navigation;
        assert(plugin_ui_navigation_category(navigation) == plugin_ui_navigation_category_state);
        if (plugin_ui_navigation_state_op(navigation) == plugin_ui_navigation_state_op_back) {
            to_state->m_next_state = plugin_ui_state_node_process_state(plugin_ui_state_node_prev(cur_state_node));
        }
        else {
            to_state->m_next_state = plugin_ui_navigation_state_to(navigation);
        }
    }

    next_control_name = plugin_ui_navigation_trigger_control(to_state->m_last_navigation);
    assert(next_control_name);

    o_action->m_type = plugin_ui_exec_action_click;
    o_action->m_click.m_click_control = plugin_ui_control_find_by_path(step->m_plan->m_env, next_control_name);
    if (o_action->m_click.m_click_control == NULL) {
        if (plugin_ui_state_node_state(cur_state_node) != plugin_ui_state_node_state_processing) {
            return plugin_ui_exec_step_retry;
        }
        else {
            CPE_ERROR(
                module->m_em, "plugin_ui_exec_step_to_state_exec: phase %s search path from state %s to %s, navigation trigger control %s not exist!",
                plugin_ui_phase_name(plugin_ui_phase_node_process_phase(to_state->m_cur_phase)),
                plugin_ui_state_node_name(cur_state_node),
                plugin_ui_state_name(to_state->m_target_state),
                next_control_name);
            return plugin_ui_exec_step_fail;
        }
    }
        
    return plugin_ui_exec_step_success;
}
