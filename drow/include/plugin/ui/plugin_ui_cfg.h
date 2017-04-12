#ifndef DROW_PLUGIN_UI_CFG_H
#define DROW_PLUGIN_UI_CFG_H
#include "render/utils/ui_vector_2.h"
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*button*/
struct plugin_ui_cfg_button {
    struct ui_vector_2 m_pos_adj;
    struct ui_vector_2 m_scale_adj;
    float m_down_duration;
    char m_down_decorator[64];
    float m_raise_duration;
    char m_raise_decorator[64];
};

ui_runtime_render_second_color_t plugin_ui_env_cfg_down_color(plugin_ui_env_t env);
    
plugin_ui_cfg_button_t plugin_ui_env_cfg_button(plugin_ui_env_t env, uint8_t ui_control_type);
void plugin_ui_env_set_cfg_button(plugin_ui_env_t env, uint8_t ui_control_type, plugin_ui_cfg_button_t cfg);    

#ifdef __cplusplus
}
#endif

#endif

