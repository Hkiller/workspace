#ifndef DROW_PLUGIN_BASICANIM_COLOR_H
#define DROW_PLUGIN_BASICANIM_COLOR_H
#include "plugin_basicanim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_color_t plugin_basicanim_color_color(plugin_basicanim_color_t anim_color);
void plugin_basicanim_color_set_color(plugin_basicanim_color_t anim_color,  ui_color_t color);    

ui_rect_t plugin_basicanim_color_rect(plugin_basicanim_color_t anim_color);
void plugin_basicanim_color_set_rect(plugin_basicanim_color_t anim_color,  ui_rect_t rect);
    
#ifdef __cplusplus
}
#endif

#endif
