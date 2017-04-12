#ifndef DROW_PLUGIN_UI_TMPL_RENDER_H
#define DROW_PLUGIN_UI_TMPL_RENDER_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_ui_control_t plugin_ui_template_render_control(plugin_ui_template_render_t render);

uint8_t plugin_ui_template_render_is_free(plugin_ui_template_render_t render);
void plugin_ui_template_render_set_is_free(plugin_ui_template_render_t render, uint8_t is_free);
    
#ifdef __cplusplus
}
#endif

#endif

