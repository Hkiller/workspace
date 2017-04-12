#ifndef PLUGIN_LAYOUT_LAYOUT_RICH_I_H
#define PLUGIN_LAYOUT_LAYOUT_RICH_I_H
#include "plugin/layout/plugin_layout_font_info.h"
#include "plugin/layout/plugin_layout_layout_rich.h"
#include "plugin_layout_layout_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_layout_layout_rich {
    struct plugin_layout_font_id m_default_font_id;
    struct plugin_layout_font_draw m_default_font_draw;
    uint8_t m_line_break;
    plugin_layout_align_t m_align;
    plugin_layout_layout_rich_block_list_t m_blocks;
};

int plugin_layout_layout_rich_register(plugin_layout_module_t module); 
void plugin_layout_layout_rich_unregister(plugin_layout_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
