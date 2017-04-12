#ifndef PLUGIN_LAYOUT_LAYOUT_BASIC_I_H
#define PLUGIN_LAYOUT_LAYOUT_BASIC_I_H
#include "render/utils/ui_color.h"
#include "plugin/layout/plugin_layout_font_info.h"
#include "plugin/layout/plugin_layout_layout_basic.h"
#include "plugin_layout_layout_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_layout_layout_basic {
    struct plugin_layout_font_id m_font_id;
    struct plugin_layout_font_draw m_font_draw;
    uint8_t m_line_break;
    plugin_layout_align_t m_align;
};

int plugin_layout_layout_basic_register(plugin_layout_module_t module); 
void plugin_layout_layout_basic_unregister(plugin_layout_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
