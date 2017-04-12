#ifndef PLUGIN_UI_CONTROL_BINDING_I_H
#define PLUGIN_UI_CONTROL_BINDING_I_H
#include "plugin/ui/plugin_ui_control_binding.h"
#include "plugin_ui_control_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_control_binding {
    plugin_ui_control_t m_control;
    TAILQ_ENTRY(plugin_ui_control_binding) m_next_for_control;
    TAILQ_ENTRY(plugin_ui_control_binding) m_next_for_page;
    uint8_t m_need_process;
    plugin_ui_control_attr_meta_t m_attr_meta;
    plugin_ui_control_binding_use_slot_list_t m_slots;
    char * m_function;
};

void plugin_ui_control_binding_real_free(plugin_ui_control_binding_t binding);

#ifdef __cplusplus
}
#endif

#endif
