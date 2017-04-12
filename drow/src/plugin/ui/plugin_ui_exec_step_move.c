#include <assert.h>
#include "plugin/ui/plugin_ui_aspect.h"
#include "plugin/ui/plugin_ui_control.h"
#include "plugin/ui/plugin_ui_control_action.h"
#include "plugin_ui_exec_step_move_i.h"

static plugin_ui_exec_step_result_t
plugin_ui_exec_step_move_exec(plugin_ui_exec_step_t step, plugin_ui_exec_action_t o_action);

plugin_ui_exec_step_move_t
plugin_ui_exec_step_move_create(plugin_ui_exec_plan_t plan, const char * move_control, const char * to_control) {
    plugin_ui_module_t module = plan->m_env->m_module;
    plugin_ui_exec_step_t step;
    plugin_ui_exec_step_move_t move;
    size_t move_control_len = strlen(move_control) + 1;
    size_t to_control_len = strlen(to_control) + 1;
    
    step = plugin_ui_exec_step_create(
        plan, sizeof(struct plugin_ui_exec_step_move) + move_control_len + to_control_len,
        plugin_ui_exec_step_move_exec);
    if (step == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_exec_step_move_create: create step fail!");
        return NULL;
    }

    move = (plugin_ui_exec_step_move_t)plugin_ui_exec_step_data(step);
    memcpy(move + 1, move_control, move_control_len);
    move->m_move_control = (void*)(move + 1);
    memcpy((void*)(move->m_move_control + move_control_len), to_control, to_control_len);
    move->m_to_control = move->m_move_control + move_control_len;
    move->m_is_error = 0;
    move->m_is_done = 0;
    
    return move;
}

static void plugin_ui_exec_step_move_on_move_done(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    plugin_ui_exec_step_t step = plugin_ui_exec_step_from_data(ctx);
    plugin_ui_module_t module = step->m_plan->m_env->m_module;
    plugin_ui_exec_step_move_t move = ctx;
    plugin_ui_control_t to_control;
    ui_rect move_rect;
    ui_rect target_rect;
    
    to_control = plugin_ui_control_find_by_path(step->m_plan->m_env, move->m_to_control);
    if (to_control == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_exec_step_move_on_move_done: to control %s not exist!", move->m_to_control);
        move->m_is_error = 1;
        return;
    }

    move_rect = plugin_ui_control_real_rt_abs(from_control);
    target_rect = plugin_ui_control_real_rt_abs(to_control);

    if (ui_rect_is_intersection_valid(&move_rect, &target_rect)) {
        move->m_is_done = 1;
    }
}

plugin_ui_exec_step_result_t
plugin_ui_exec_step_move_exec(plugin_ui_exec_step_t step, plugin_ui_exec_action_t o_action) {
    plugin_ui_module_t module = step->m_plan->m_env->m_module;
    plugin_ui_exec_step_move_t move;
    plugin_ui_aspect_t aspect;
    plugin_ui_control_t to_control;
    plugin_ui_control_t move_control;

    move = (plugin_ui_exec_step_move_t)plugin_ui_exec_step_data(step);
    if (move->m_is_done) return plugin_ui_exec_step_done;
    if (move->m_is_error) return plugin_ui_exec_step_fail;
    
    aspect = plugin_ui_exec_plan_aspect(step->m_plan);
    if (aspect == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_exec_step_move_exec: no aspect!");
        return plugin_ui_exec_step_fail;
    }
    
    to_control = plugin_ui_control_find_by_path(step->m_plan->m_env, move->m_to_control);
    if (to_control == NULL) {
        o_action->m_type = plugin_ui_exec_action_noop;
        return plugin_ui_exec_step_success;
    }

    move_control = plugin_ui_control_find_by_path(step->m_plan->m_env, move->m_move_control);
    if (move_control == NULL) {
        o_action->m_type = plugin_ui_exec_action_noop;
        return plugin_ui_exec_step_success;
    }
    
    if (!plugin_ui_aspect_control_has_action_in(aspect, move_control)) {
        plugin_ui_control_action_t action =
            plugin_ui_control_action_create(
                move_control, plugin_ui_event_move_done,
                plugin_ui_event_scope_self, plugin_ui_exec_step_move_on_move_done, move);
        if (action == NULL) {
            CPE_ERROR(module->m_em, "plugin_ui_exec_step_move_exec: create action fail!");
            return plugin_ui_exec_step_fail;
        }

        if (plugin_ui_aspect_control_action_add(aspect, action, 1) != 0) {
            CPE_ERROR(module->m_em, "plugin_ui_exec_step_move_exec: add action to aspect fail!");
            return plugin_ui_exec_step_fail;
        }
    }

    o_action->m_type = plugin_ui_exec_action_move;
    o_action->m_move.m_to_control = to_control;
    o_action->m_move.m_move_control = move_control;
    
    return plugin_ui_exec_step_success;
}
