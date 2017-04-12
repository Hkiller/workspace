#ifndef PLUGIN_UI_PAGE_EH_I_H
#define PLUGIN_UI_PAGE_EH_I_H
#include "plugin/ui/plugin_ui_page_eh.h"
#include "plugin_ui_page_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_page_eh {
    plugin_ui_page_t m_page;
    TAILQ_ENTRY(plugin_ui_page_eh) m_next;
    uint8_t m_is_processing;
    uint8_t m_is_free;
    uint8_t m_is_active;
    const char * m_event;
    plugin_ui_page_eh_scope_t m_scope;
    plugin_ui_page_eh_fun_t m_fun;
    void * m_ctx;
};

void plugin_ui_page_eh_real_free(plugin_ui_page_eh_t eh);
int plugin_ui_page_eh_sync_active(plugin_ui_page_eh_t eh);

#ifdef __cplusplus
}
#endif

#endif
