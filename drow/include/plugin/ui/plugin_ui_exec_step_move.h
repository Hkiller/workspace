#ifndef PLUGIN_UI_EXEC_STEP_MOVE_H
#define PLUGIN_UI_EXEC_STEP_MOVE_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_ui_exec_step_move_t
plugin_ui_exec_step_move_create(
    plugin_ui_exec_plan_t plan, const char * move_control, const char * to_control);

#ifdef __cplusplus
}
#endif

#endif

