#ifndef PLUGIN_UI_MOUSE_I_H
#define PLUGIN_UI_MOUSE_I_H
#include "plugin/ui/plugin_ui_mouse.h"
#include "plugin_ui_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_mouse {
    plugin_ui_env_t m_env;
    uint8_t m_is_active;
    ui_vector_2 m_cur_pt;
    uint8_t m_l_down;
    uint8_t m_r_down;
};

plugin_ui_mouse_t plugin_ui_mouse_create(plugin_ui_env_t env);
void plugin_ui_mouse_free(plugin_ui_mouse_t mouse);

#ifdef __cplusplus
}
#endif

#endif
