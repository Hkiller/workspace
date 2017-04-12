#ifndef PLUGIN_UI_POPUP_I_H
#define PLUGIN_UI_POPUP_I_H
#include "plugin/ui/plugin_ui_popup.h"
#include "plugin_ui_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_popup {
    uint32_t m_id;
    plugin_ui_env_t m_env;
    TAILQ_ENTRY(plugin_ui_popup) m_next_for_env;
    plugin_ui_page_t m_create_from_page;
    TAILQ_ENTRY(plugin_ui_popup) m_next_for_create_from_page;
    plugin_ui_popup_def_t m_def;
    
    char m_name[64];
    int16_t m_layer;
    plugin_ui_popup_action_list_t m_actions;
    plugin_ui_page_t m_page;
    float m_lifecircle;
    void * m_allock_data_buf;
    char m_data_buf[128];
};

void plugin_ui_popup_real_free(plugin_ui_popup_t popup);
void plugin_ui_popup_adj_layer(plugin_ui_popup_t popup);

#ifdef __cplusplus
}
#endif

#endif
