#ifndef PLUGIN_UI_EXEC_STEP_I_H
#define PLUGIN_UI_EXEC_STEP_I_H
#include "plugin/ui/plugin_ui_exec_step.h"
#include "plugin_ui_exec_plan_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_exec_step {
    plugin_ui_exec_plan_t m_plan;
    TAILQ_ENTRY(plugin_ui_exec_step) m_next;
    plugin_ui_exec_step_fini_fun_t m_fini;
    plugin_ui_exec_step_exec_fun_t m_exec;
    size_t m_capacity;
};

#ifdef __cplusplus
}
#endif

#endif
