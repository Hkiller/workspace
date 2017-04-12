#ifndef PLUGIN_UI_STATE_NODE_PAGE_I_H
#define PLUGIN_UI_STATE_NODE_PAGE_I_H
#include "plugin_ui_state_node_i.h"
#include "plugin_ui_page_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_state_node_page {
    plugin_ui_state_node_t m_state_node;
    TAILQ_ENTRY(plugin_ui_state_node_page) m_next_for_state_node;
    plugin_ui_page_t m_page;
    TAILQ_ENTRY(plugin_ui_state_node_page) m_next_for_page;
    char m_before_page[64];
    plugin_ui_page_state_on_hide_fun_t m_on_hide_fun;
    void * m_on_hide_ctx;
};

plugin_ui_state_node_page_t
plugin_ui_state_node_page_create(
    plugin_ui_state_node_t state_node, plugin_ui_page_t page, const char * before_page,
    plugin_ui_page_state_on_hide_fun_t on_hide_fun, 
    void * m_on_hide_ctx);

plugin_ui_state_node_page_t
plugin_ui_state_node_page_find(plugin_ui_state_node_t state_node, plugin_ui_page_t page);
    
void plugin_ui_state_node_page_free(plugin_ui_state_node_page_t state_node_page);
void plugin_ui_state_node_page_real_free(plugin_ui_state_node_page_t state_node_page);    

#ifdef __cplusplus
}
#endif

#endif
