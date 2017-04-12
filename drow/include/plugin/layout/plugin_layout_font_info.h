#ifndef PLUGIN_LAYOUT_FONT_DATA_H
#define PLUGIN_LAYOUT_FONT_DATA_H
#include "render/utils/ui_color.h"
#include "plugin_layout_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_layout_font_id {
	uint8_t category; /*see plugin_layout_font_category*/
	uint8_t face;
	uint8_t size;
	uint8_t stroke_width;
};

struct plugin_layout_font_draw {
    uint8_t flag; /*see plugin_layout_font_draw_flag*/
    ui_color color;
    ui_color stroke_color;
	uint32_t grap_horz;
	uint32_t grap_vert;
};
        
#ifdef __cplusplus
}
#endif

#endif
