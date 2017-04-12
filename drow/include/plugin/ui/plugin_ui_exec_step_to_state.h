#ifndef PLUGIN_UI_EXEC_STEP_TO_STATE_H
#define PLUGIN_UI_EXEC_STEP_TO_STATE_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_ui_exec_step_to_state_t plugin_ui_exec_step_to_state_create(plugin_ui_exec_plan_t plan, plugin_ui_state_t target_state);

#ifdef __cplusplus
}
#endif

#endif

