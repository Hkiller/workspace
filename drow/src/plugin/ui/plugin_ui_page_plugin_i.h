#ifndef PLUGIN_UI_PAGE_SETUP_I_H
#define PLUGIN_UI_PAGE_SETUP_I_H
#include "plugin/ui/plugin_ui_page_plugin.h"
#include "plugin_ui_page_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_page_plugin {
    plugin_ui_page_t m_page;
    TAILQ_ENTRY(plugin_ui_page_plugin) m_next_for_page;
    TAILQ_ENTRY(plugin_ui_page_plugin) m_next_for_env;
    plugin_ui_page_plugin_on_init_t m_on_init;
    plugin_ui_page_plugin_on_fini_t m_on_fini;
    plugin_ui_page_plugin_on_load_t m_on_load;
    plugin_ui_page_plugin_on_unload_t m_on_unload;
    plugin_ui_page_plugin_on_visiable_t m_on_visiable;
    plugin_ui_page_plugin_on_hide_t m_on_hide;
    void * m_ctx;
    size_t m_capacity;
    uint8_t m_is_loaded;
};

#ifdef __cplusplus
}
#endif

#endif
