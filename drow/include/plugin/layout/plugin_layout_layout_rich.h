#ifndef DROW_LAYOUT_LAYOUT_RICH_H
#define DROW_LAYOUT_LAYOUT_RICH_H
#include "plugin_layout_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_layout_font_id_t plugin_layout_layout_rich_default_font_id(plugin_layout_layout_rich_t);
void plugin_layout_layout_rich_set_default_font_id(plugin_layout_layout_rich_t rich, plugin_layout_font_id_t font_id);

plugin_layout_font_draw_t plugin_layout_layout_rich_default_font_draw(plugin_layout_layout_rich_t rich);
void plugin_layout_layout_rich_set_default_font_draw(plugin_layout_layout_rich_t rich, plugin_layout_font_draw_t font_draw);    

plugin_layout_align_t plugin_layout_layout_rich_align(plugin_layout_layout_rich_t rich);
void plugin_layout_layout_rich_set_align(plugin_layout_layout_rich_t rich, plugin_layout_align_t align);
    
uint8_t plugin_layout_layout_rich_line_break(plugin_layout_layout_rich_t rich);
void plugin_layout_layout_rich_set_line_break(plugin_layout_layout_rich_t rich, uint8_t line_break);

void plugin_layout_layout_rich_clear_blocks(plugin_layout_layout_rich_t rich);
    
#ifdef __cplusplus
}
#endif

#endif

