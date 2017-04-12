#ifndef PLUGIN_UI_ANIMATION_CONTROL_I_H
#define PLUGIN_UI_ANIMATION_CONTROL_I_H
#include "plugin/ui/plugin_ui_animation_control.h"
#include "plugin_ui_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif
    
struct plugin_ui_animation_control {
    plugin_ui_animation_t m_animation;
    TAILQ_ENTRY(plugin_ui_animation_control) m_next_for_animation;
    plugin_ui_control_t m_control;
    TAILQ_ENTRY(plugin_ui_animation_control) m_next_for_control;
    uint8_t m_is_tie;
};

void plugin_ui_animation_control_real_free(plugin_ui_animation_control_t animation_control);
    
#ifdef __cplusplus
}
#endif

#endif
