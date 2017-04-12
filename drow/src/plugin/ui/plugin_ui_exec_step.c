#include "plugin_ui_exec_step_i.h"

plugin_ui_exec_step_t
plugin_ui_exec_step_create(plugin_ui_exec_plan_t exec_plan, size_t capacity, plugin_ui_exec_step_exec_fun_t exec) {
    plugin_ui_exec_step_t step;

    step = mem_alloc(exec_plan->m_env->m_module->m_alloc, sizeof(struct plugin_ui_exec_plan) + capacity);
    if (step == NULL) {
        CPE_ERROR(exec_plan->m_env->m_module->m_em, "plugin_ui_exec_step_create: alloc fail!");
        return NULL;
    }

    step->m_plan = exec_plan;
    step->m_fini = NULL;
    step->m_exec = exec;
    step->m_capacity = capacity;
    
    TAILQ_INSERT_TAIL(&exec_plan->m_steps, step, m_next);
    if (exec_plan->m_cur_step == NULL) {
        exec_plan->m_cur_step = step;
    }

    return step;
}

void plugin_ui_exec_step_free(plugin_ui_exec_step_t exec_step) {
    plugin_ui_exec_plan_t exec_plan = exec_step->m_plan;

    if (exec_step->m_fini) exec_step->m_fini(exec_step);
    
    if (exec_plan->m_cur_step == exec_step) {
        exec_plan->m_last_step_result = plugin_ui_exec_step_success;
        exec_plan->m_cur_step = TAILQ_NEXT(exec_step, m_next);
    }

    TAILQ_REMOVE(&exec_plan->m_steps, exec_step, m_next);

    mem_free(exec_plan->m_env->m_module->m_alloc, exec_step);
}

void plugin_ui_exec_step_set_fini(plugin_ui_exec_step_t exec_step, plugin_ui_exec_step_fini_fun_t fini) {
    exec_step->m_fini = fini;
}

void * plugin_ui_exec_step_data(plugin_ui_exec_step_t exec_step) {
    return exec_step + 1;
}

plugin_ui_exec_step_t plugin_ui_exec_step_from_data(void * data) {
    return ((plugin_ui_exec_step_t)data) - 1;
}
