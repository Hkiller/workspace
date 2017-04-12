#ifndef PLUGIN_UI_POPUP_DEF_I_H
#define PLUGIN_UI_POPUP_DEF_I_H
#include "plugin/ui/plugin_ui_popup_def.h"
#include "plugin_ui_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_popup_def {
    plugin_ui_env_t m_env;
    TAILQ_ENTRY(plugin_ui_popup_def) m_next;
    plugin_ui_phase_use_popup_def_list_t m_used_by_phases;
    char m_name[64];
    plugin_ui_page_meta_t m_page_meta;
    LPDRMETA m_data_meta;
    char * m_page_load_from;
    int16_t m_layer;
    float m_lifecircle;
    plugin_ui_popup_def_binding_list_t m_bindings;
};

#ifdef __cplusplus
}
#endif

#endif
