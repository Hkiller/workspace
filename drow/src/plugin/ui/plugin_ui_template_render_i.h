#ifndef PLUGIN_UI_TEMPLATE_RENDER_I_H
#define PLUGIN_UI_TEMPLATE_RENDER_I_H
#include "plugin/ui/plugin_ui_template_render.h"
#include "plugin_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_template_render {
    plugin_ui_control_t m_control;
    uint8_t m_is_free;
    uint8_t m_pos_policy;
};

int plugin_ui_template_render_regist(plugin_ui_module_t module);
void plugin_ui_template_render_unregist(plugin_ui_module_t module);

#ifdef __cplusplus
}
#endif

#endif
