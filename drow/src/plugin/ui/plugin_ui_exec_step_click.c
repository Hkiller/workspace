#include <assert.h>
#include "plugin/ui/plugin_ui_aspect.h"
#include "plugin/ui/plugin_ui_control.h"
#include "plugin/ui/plugin_ui_control_action.h"
#include "plugin_ui_exec_step_click_i.h"

static plugin_ui_exec_step_result_t
plugin_ui_exec_step_click_exec(plugin_ui_exec_step_t step, plugin_ui_exec_action_t o_action);

plugin_ui_exec_step_click_t
plugin_ui_exec_step_click_create(plugin_ui_exec_plan_t plan, const char * action) {
    plugin_ui_module_t module = plan->m_env->m_module;
    plugin_ui_exec_step_t step;
    plugin_ui_exec_step_click_t click;
    size_t action_len = strlen(action) + 1;
    
    step = plugin_ui_exec_step_create(plan, sizeof(struct plugin_ui_exec_step_click) + action_len, plugin_ui_exec_step_click_exec);
    if (step == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_exec_step_click_create: create step fail!");
        return NULL;
    }

    click = (plugin_ui_exec_step_click_t)plugin_ui_exec_step_data(step);
    memcpy(click + 1, action, action_len);
    click->m_action = (void*)(click + 1);
    click->m_is_done = 0;
    
    return click;
}

static void plugin_ui_exec_step_click_on_click(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    plugin_ui_exec_step_click_t click = ctx;
    click->m_is_done = 1;
}

plugin_ui_exec_step_result_t
plugin_ui_exec_step_click_exec(plugin_ui_exec_step_t step, plugin_ui_exec_action_t o_action) {
    plugin_ui_module_t module = step->m_plan->m_env->m_module;
    plugin_ui_exec_step_click_t click;
    plugin_ui_aspect_t aspect;
    plugin_ui_control_t control;

    click = (plugin_ui_exec_step_click_t)plugin_ui_exec_step_data(step);
    if (click->m_is_done) return plugin_ui_exec_step_done;
    
    aspect = plugin_ui_exec_plan_aspect(step->m_plan);
    if (aspect == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_exec_step_click_exec: no aspect!");
        return plugin_ui_exec_step_fail;
    }
    
    control = plugin_ui_control_find_by_path(step->m_plan->m_env, click->m_action);
    if (control == NULL) {
        o_action->m_type = plugin_ui_exec_action_noop;
        return plugin_ui_exec_step_success;
    }
    
    if (!plugin_ui_aspect_control_has_action_in(aspect, control)) {
        plugin_ui_control_action_t action =
            plugin_ui_control_action_create(
                control, plugin_ui_event_mouse_click,
                plugin_ui_event_scope_all, plugin_ui_exec_step_click_on_click, click);
        if (action == NULL) {
            CPE_ERROR(module->m_em, "plugin_ui_exec_step_click_exec: create action fail!");
            return plugin_ui_exec_step_fail;
        }

        if (plugin_ui_aspect_control_action_add(aspect, action, 1) != 0) {
            CPE_ERROR(module->m_em, "plugin_ui_exec_step_click_exec: add action to aspect fail!");
            return plugin_ui_exec_step_fail;
        }
    }

    o_action->m_type = plugin_ui_exec_action_click;
    o_action->m_click.m_click_control = control;
    
    return plugin_ui_exec_step_success;
}
