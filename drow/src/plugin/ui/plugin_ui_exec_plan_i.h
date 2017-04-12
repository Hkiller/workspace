#ifndef PLUGIN_UI_EXEC_PLAN_I_H
#define PLUGIN_UI_EXEC_PLAN_I_H
#include "plugin/ui/plugin_ui_exec_plan.h"
#include "plugin/ui/plugin_ui_exec_step.h"
#include "plugin_ui_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_exec_plan {
    plugin_ui_env_t m_env;
    struct plugin_ui_exec_action m_next_action;
    plugin_ui_exec_step_t m_cur_step;
    plugin_ui_exec_step_result_t m_last_step_result;
    plugin_ui_exec_step_list_t m_steps;
    plugin_ui_aspect_t m_aspect;
    plugin_ui_popup_t m_base_popup;
};

#ifdef __cplusplus
}
#endif

#endif
