#ifndef PLUGIN_UI_EXEC_STEP_H
#define PLUGIN_UI_EXEC_STEP_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum plugin_ui_exec_step_result {
    plugin_ui_exec_step_fail,
    plugin_ui_exec_step_retry,
    plugin_ui_exec_step_success,
    plugin_ui_exec_step_done,
} plugin_ui_exec_step_result_t;

typedef void (*plugin_ui_exec_step_fini_fun_t)(plugin_ui_exec_step_t step);
typedef plugin_ui_exec_step_result_t (*plugin_ui_exec_step_exec_fun_t)(plugin_ui_exec_step_t step, plugin_ui_exec_action_t o_action);
    
plugin_ui_exec_step_t plugin_ui_exec_step_create(plugin_ui_exec_plan_t plan, size_t capacity, plugin_ui_exec_step_exec_fun_t exec);
void plugin_ui_exec_step_free(plugin_ui_exec_step_t exec_step);

void plugin_ui_exec_step_set_fini(plugin_ui_exec_step_t exec_step, plugin_ui_exec_step_fini_fun_t fini);
    
void * plugin_ui_exec_step_data(plugin_ui_exec_step_t exec_step);
plugin_ui_exec_step_t plugin_ui_exec_step_from_data(void * data);

#ifdef __cplusplus
}
#endif

#endif

