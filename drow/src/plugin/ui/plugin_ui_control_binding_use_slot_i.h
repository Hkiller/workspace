#ifndef PLUGIN_UI_CONTROL_BINDING_USE_SLOT_I_H
#define PLUGIN_UI_CONTROL_BINDING_USE_SLOT_I_H
#include "plugin_ui_control_binding_i.h"
#include "plugin_ui_page_slot_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_control_binding_use_slot {
    plugin_ui_control_binding_t m_binding;
    TAILQ_ENTRY(plugin_ui_control_binding_use_slot) m_next_for_binding;
    plugin_ui_page_slot_t m_slot;
    TAILQ_ENTRY(plugin_ui_control_binding_use_slot) m_next_for_slot;
};

plugin_ui_control_binding_use_slot_t
plugin_ui_control_binding_use_slot_create(plugin_ui_control_binding_t binding, plugin_ui_page_slot_t slot);

void plugin_ui_control_binding_use_slot_free(plugin_ui_control_binding_use_slot_t binding_use_slot);
    
void plugin_ui_control_binding_use_slot_real_free(plugin_ui_control_binding_use_slot_t binding_use_slot);
    
#ifdef __cplusplus
}
#endif

#endif
