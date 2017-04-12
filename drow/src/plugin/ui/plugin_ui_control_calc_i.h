#ifndef PLUGIN_UI_CONTROL_CALC_I_H
#define PLUGIN_UI_CONTROL_CALC_I_H
#include "plugin/ui/plugin_ui_control_calc.h"
#include "plugin_ui_control_i.h"

#ifdef __cplusplus
extern "C" {
#endif

xtoken_t plugin_ui_control_calc_i(plugin_ui_control_t control, const char * def, dr_data_source_t data_source);

#ifdef __cplusplus
}
#endif

#endif
