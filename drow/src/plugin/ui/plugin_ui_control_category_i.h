#ifndef PLUGIN_UI_CONTROL_CATEGORY_I_H
#define PLUGIN_UI_CONTROL_CATEGORY_I_H
#include "plugin/ui/plugin_ui_control_category.h"
#include "plugin_ui_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_control_category {
    plugin_ui_env_t m_env;
    TAILQ_ENTRY(plugin_ui_control_category) m_next;
    
    char m_prefix[64];
    char * m_click_audio;
};

#ifdef __cplusplus
}
#endif

#endif
