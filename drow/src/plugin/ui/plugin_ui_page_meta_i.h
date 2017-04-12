#ifndef PLUGIN_UI_PAGE_META_I_H
#define PLUGIN_UI_PAGE_META_I_H
#include "plugin/ui/plugin_ui_page_meta.h"
#include "plugin_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_page_meta {
    plugin_ui_module_t m_module;
    const char * m_name;
    struct cpe_hash_entry m_hh_for_module;
    
    uint32_t m_data_capacity;
    void * m_ctx;
    plugin_ui_page_init_fun_t m_init;
    plugin_ui_page_fini_fun_t m_fini;
    plugin_ui_page_update_fun_t m_on_update;
    plugin_ui_page_event_fun_t m_on_changed;
    plugin_ui_page_event_fun_t m_on_hide;
    plugin_ui_page_event_fun_t m_on_load;

    uint16_t m_page_count;
    plugin_ui_page_list_t m_pages;
};

void plugin_ui_page_meta_free_all(const plugin_ui_module_t module);
    
uint32_t plugin_ui_page_meta_hash(const plugin_ui_page_meta_t meta);
int plugin_ui_page_meta_eq(const plugin_ui_page_meta_t l, const plugin_ui_page_meta_t r);
    
#ifdef __cplusplus
}
#endif

#endif
