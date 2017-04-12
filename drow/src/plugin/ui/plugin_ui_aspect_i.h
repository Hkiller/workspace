#ifndef PLUGIN_UI_ASPECT_I_H
#define PLUGIN_UI_ASPECT_I_H
#include "plugin/ui/plugin_ui_aspect.h"
#include "plugin_ui_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_aspect {
    plugin_ui_env_t m_env;
    TAILQ_ENTRY(plugin_ui_aspect) m_next;
    char m_name[64];
    plugin_ui_aspect_ref_list_t m_env_actions;
    plugin_ui_aspect_ref_list_t m_controls;
    plugin_ui_aspect_ref_list_t m_animations;
    plugin_ui_aspect_ref_list_t m_control_actions;
    plugin_ui_aspect_ref_list_t m_control_frames;
};

void plugin_ui_aspect_real_free(plugin_ui_aspect_t aspect);

#ifdef __cplusplus
}
#endif

#endif
