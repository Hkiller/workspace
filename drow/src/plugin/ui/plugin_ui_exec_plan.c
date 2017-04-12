#include <assert.h>
#include "plugin/ui/plugin_ui_aspect.h"
#include "plugin_ui_exec_plan_i.h"
#include "plugin_ui_exec_step_i.h"
#include "plugin_ui_popup_i.h"

plugin_ui_exec_plan_t plugin_ui_exec_plan_create(plugin_ui_env_t env) {
    plugin_ui_exec_plan_t exec_plan;

    exec_plan = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_exec_plan));
    if (exec_plan == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_exec_plan: alloc fail!");
        return NULL;
    }

    exec_plan->m_env = env;
    exec_plan->m_cur_step = NULL;
    exec_plan->m_last_step_result = plugin_ui_exec_step_success;
    exec_plan->m_aspect = NULL;
    exec_plan->m_base_popup = NULL;
    TAILQ_INIT(&exec_plan->m_steps);
    
    return exec_plan;
}

void plugin_ui_exec_plan_free(plugin_ui_exec_plan_t exec_plan) {
    plugin_ui_env_t env = exec_plan->m_env;

    while(!TAILQ_EMPTY(&exec_plan->m_steps)) {
        plugin_ui_exec_step_free(TAILQ_FIRST(&exec_plan->m_steps));
    }
    assert(exec_plan->m_cur_step == NULL);

    if (exec_plan->m_aspect) {
        plugin_ui_aspect_free(exec_plan->m_aspect);
        exec_plan->m_aspect = NULL;
    }
    
    mem_free(env->m_module->m_alloc, exec_plan);
}

plugin_ui_aspect_t plugin_ui_exec_plan_aspect(plugin_ui_exec_plan_t exec_plan) {
    if (exec_plan->m_aspect == NULL) {
        exec_plan->m_aspect = plugin_ui_aspect_create(exec_plan->m_env, NULL);
    }
    return exec_plan->m_aspect;
}

plugin_ui_popup_t plugin_ui_exec_plan_base_popup(plugin_ui_exec_plan_t exec_plan) {
    return exec_plan->m_base_popup;
}

void plugin_ui_exec_plan_set_base_popup(plugin_ui_exec_plan_t exec_plan, plugin_ui_popup_t popup) {
    exec_plan->m_base_popup = popup;
}

static plugin_ui_exec_step_result_t
plugin_ui_exec_plan_check_popups(plugin_ui_exec_plan_t exec_plan, plugin_ui_exec_action_t o_action) {
    plugin_ui_popup_t popup;

    for(popup = exec_plan->m_base_popup
            ? TAILQ_PREV(exec_plan->m_base_popup, plugin_ui_popup_list, m_next_for_env)
            : TAILQ_LAST(&exec_plan->m_env->m_popups, plugin_ui_popup_list);
        popup != TAILQ_END(&exec_plan->m_env->m_popups);
        popup = TAILQ_PREV(popup, plugin_ui_popup_list, m_next_for_env))
    {
        plugin_ui_control_t control = plugin_ui_popup_find_action_control(popup, "ok");
        if (control) {
            o_action->m_type = plugin_ui_exec_action_click;
            o_action->m_click.m_click_control = control;
            return plugin_ui_exec_step_success;
        }
    }
    
    return plugin_ui_exec_step_done;
}

plugin_ui_exec_action_t
plugin_ui_exec_plan_next_action(plugin_ui_exec_plan_t exec_plan) {
PLUGIN_UI_EXEC_PLAN_CHECK_AGAIN:
    if (exec_plan->m_cur_step == NULL) return NULL;

    switch (exec_plan->m_last_step_result) {
    case plugin_ui_exec_step_fail:
        return NULL;
    case plugin_ui_exec_step_success:
        /*检查popups */
        exec_plan->m_last_step_result = plugin_ui_exec_plan_check_popups(exec_plan, &exec_plan->m_next_action);
        switch(exec_plan->m_last_step_result) {
        case plugin_ui_exec_step_fail:
            return NULL;
        case plugin_ui_exec_step_success:
            return &exec_plan->m_next_action;
        case plugin_ui_exec_step_done:
            break;
        default:
            CPE_ERROR(
                exec_plan->m_env->m_module->m_em, "plugin_ui_exec_plan_next_action: check popups result %d fail!",
                exec_plan->m_last_step_result);
            return NULL;
        }

        /*go throw*/
    case plugin_ui_exec_step_retry:
        /*执行下一个步骤 */
        exec_plan->m_last_step_result = exec_plan->m_cur_step->m_exec(exec_plan->m_cur_step, &exec_plan->m_next_action);
        switch(exec_plan->m_last_step_result) {
        case plugin_ui_exec_step_fail:
        case plugin_ui_exec_step_retry:
            return NULL;
        case plugin_ui_exec_step_success:
            return &exec_plan->m_next_action;
        case plugin_ui_exec_step_done:
            exec_plan->m_cur_step = TAILQ_NEXT(exec_plan->m_cur_step, m_next);
            exec_plan->m_last_step_result = plugin_ui_exec_step_success;
                
            goto PLUGIN_UI_EXEC_PLAN_CHECK_AGAIN;
        default:
            CPE_ERROR(
                exec_plan->m_env->m_module->m_em, "plugin_ui_exec_plan_next_action: last step result %d fail!",
                exec_plan->m_last_step_result);
            return NULL;
        }
    case plugin_ui_exec_step_done:
        CPE_ERROR(exec_plan->m_env->m_module->m_em, "plugin_ui_exec_plan_next_action: last step result unexpect result done!");
        return NULL;
    default:
        CPE_ERROR(
            exec_plan->m_env->m_module->m_em, "plugin_ui_exec_plan_next_action: last step result %d unknown!",
            exec_plan->m_last_step_result);
        return NULL;
    }
}

plugin_ui_exec_state_type_t
plugin_ui_exec_plan_state(plugin_ui_exec_plan_t exec_plan) {
    if (exec_plan->m_cur_step == NULL) return plugin_ui_exec_success;
    if (exec_plan->m_last_step_result == plugin_ui_exec_step_fail) return plugin_ui_exec_fail;
    return plugin_ui_exec_processing;
}
