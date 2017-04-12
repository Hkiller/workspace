#ifndef PLUGIN_UI_CONTROL_TIMER_I_H
#define PLUGIN_UI_CONTROL_TIMER_I_H
#include "plugin/ui/plugin_ui_control_timer.h"
#include "plugin_ui_control_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_control_timer {
    plugin_ui_control_t m_control;
    TAILQ_ENTRY(plugin_ui_control_timer) m_next;
    uint16_t m_timer_type;
    uint16_t m_repeat_count;
    uint32_t m_next_left;
    uint32_t m_span;
    plugin_ui_timer_fun_t m_fun;
    void * m_ctx;
    char m_data[PLUGIN_UI_CONTROL_ACTION_DATA_CAPACITY];
};

void plugin_ui_control_timer_real_free(plugin_ui_control_timer_t timer);
void plugin_ui_control_timer_place(plugin_ui_control_timer_t check_start, plugin_ui_control_timer_t timer);
void plugin_ui_control_timer_process(plugin_ui_control_t control, uint16_t duration_ms);

#ifdef __cplusplus
}
#endif

#endif
