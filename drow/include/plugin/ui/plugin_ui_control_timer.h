#ifndef DROW_PLUGIN_UI_CONTROL_TIMER_H
#define DROW_PLUGIN_UI_CONTROL_TIMER_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_ui_control_timer_t
plugin_ui_control_timer_create(
    plugin_ui_control_t control, uint16_t timer_type,
    uint32_t delay, uint32_t span, uint16_t repeat,
    plugin_ui_timer_fun_t fun, void * ctx);

void plugin_ui_control_timer_free(plugin_ui_control_timer_t timer);

uint8_t plugin_ui_control_timer_data_capacity(plugin_ui_control_timer_t timer);
void * plugin_ui_control_timer_data(plugin_ui_control_timer_t timer);

void plugin_ui_control_timer_clear(plugin_ui_control_t control);
void plugin_ui_control_timer_clear_by_type(plugin_ui_control_t control, uint16_t timer_type);
    
#ifdef __cplusplus
}
#endif

#endif

