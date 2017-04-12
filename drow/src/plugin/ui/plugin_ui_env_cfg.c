#include "plugin_ui_env_i.h"

void plugin_ui_env_cfg_init(plugin_ui_env_t env) {
    env->m_cfg_down_color.m_mix = ui_runtime_render_second_color_multiply;
    env->m_cfg_down_color.m_color.r = 136;
    env->m_cfg_down_color.m_color.g = 136;
    env->m_cfg_down_color.m_color.b = 136;
    env->m_cfg_down_color.m_color.a = 255;
    
    env->m_cfg_button.m_pos_adj.x = 4.0f;
    env->m_cfg_button.m_pos_adj.y = 4.0f;
    env->m_cfg_button.m_scale_adj = UI_VECTOR_2_IDENTITY;
    env->m_cfg_button.m_down_duration = 0.0f;
    env->m_cfg_button.m_down_decorator[0] = 0;
    env->m_cfg_button.m_raise_duration = 0.0f;
    env->m_cfg_button.m_raise_decorator[0] = 0;
    
    env->m_cfg_toggle.m_pos_adj.x = 0.0f;
    env->m_cfg_toggle.m_pos_adj.y = 0.0f;
    env->m_cfg_toggle.m_scale_adj = UI_VECTOR_2_IDENTITY;
    env->m_cfg_toggle.m_down_duration = 0.0f;
    env->m_cfg_toggle.m_down_decorator[0] = 0;
    env->m_cfg_toggle.m_raise_duration = 0.0f;
    env->m_cfg_toggle.m_raise_decorator[0] = 0;
}

ui_runtime_render_second_color_t plugin_ui_env_cfg_down_color(plugin_ui_env_t env) {
    return &env->m_cfg_down_color;
}

plugin_ui_cfg_button_t plugin_ui_env_cfg_button(plugin_ui_env_t env, uint8_t ui_control_type) {
    switch(ui_control_type) {
    case ui_control_type_toggle:
        return &env->m_cfg_toggle;
    case ui_control_type_button:
    default:
        return &env->m_cfg_button;
    }
}

void plugin_ui_env_set_cfg_button(plugin_ui_env_t env, uint8_t ui_control_type, plugin_ui_cfg_button_t cfg) {
    switch(ui_control_type) {
    case ui_control_type_button:
        env->m_cfg_button = *cfg;
        break;
    case ui_control_type_toggle:
        env->m_cfg_toggle = *cfg;
        break;
    default:
        CPE_ERROR(env->m_module->m_em, "plugin_ui_env_set_cfg_button: not support control type %d", ui_control_type);
        break;
    }
}



