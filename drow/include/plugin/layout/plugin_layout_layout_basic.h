#ifndef DROW_LAYOUT_LAYOUT_BASIC_H
#define DROW_LAYOUT_LAYOUT_BASIC_H
#include "plugin_layout_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_layout_font_id_t plugin_layout_layout_basic_font_id(plugin_layout_layout_basic_t);
void plugin_layout_layout_basic_set_font_id(plugin_layout_layout_basic_t basic, plugin_layout_font_id_t font_id);

plugin_layout_font_draw_t plugin_layout_layout_basic_font_draw(plugin_layout_layout_basic_t basic);
void plugin_layout_layout_basic_set_font_draw(plugin_layout_layout_basic_t basic, plugin_layout_font_draw_t font_draw);    

plugin_layout_align_t plugin_layout_layout_basic_align(plugin_layout_layout_basic_t basic);
void plugin_layout_layout_basic_set_align(plugin_layout_layout_basic_t basic, plugin_layout_align_t align);
    
uint8_t plugin_layout_layout_basic_line_break(plugin_layout_layout_basic_t basic);
void plugin_layout_layout_basic_set_line_break(plugin_layout_layout_basic_t basic, uint8_t line_break);

#ifdef __cplusplus
}
#endif

#endif

