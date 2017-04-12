#ifndef PLUGIN_UI_PAGE_SLOT_I_H
#define PLUGIN_UI_PAGE_SLOT_I_H
#include "plugin/ui/plugin_ui_page_slot.h"
#include "plugin_ui_page_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_page_slot {
    plugin_ui_page_t m_page;
    TAILQ_ENTRY(plugin_ui_page_slot) m_next;
    char m_name[64];
    plugin_ui_control_binding_use_slot_list_t m_bindings;
    struct dr_value m_value;
    char * m_buf;
    char m_inline_buf[64];
};

void plugin_ui_page_slot_real_free(plugin_ui_page_slot_t slot);

#ifdef __cplusplus
}
#endif

#endif
