#ifndef PLUGIN_LAYOUT_FONT_META_FONT_I_H
#define PLUGIN_LAYOUT_FONT_META_FONT_I_H
#include "ft2build.h"
#include "freetype/freetype.h"
#include "hb.h"
#include "hb-ft.h"
#include "plugin/layout/plugin_layout_font_meta_font.h"
#include "plugin_layout_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_layout_font_meta_font_span {
    int16_t x, y, width;
    uint8_t coverage;
};

struct plugin_layout_font_meta_font {
    FT_Library m_lib;
    struct plugin_layout_font_meta_font_span * m_spans;
    uint16_t m_span_capacity;
    uint16_t m_span_count;
};

struct plugin_layout_font_face_font {
    ui_cache_res_t m_font_res;
    FT_Face m_face;
    hb_font_t * m_hb_font;
};

struct plugin_layout_font_element_font {
    uint8_t m_is_loaded;
    ui_rect m_outline_texture_rect;
};
    
int plugin_layout_font_meta_font_register(plugin_layout_module_t module); 
void plugin_layout_font_meta_font_unregister(plugin_layout_module_t module);

int plugin_layout_font_meta_font_basic_layout(
    void * ctx, plugin_layout_font_face_t face,
    plugin_layout_render_t render, plugin_layout_font_draw_t font_draw,
    uint32_t const * text, size_t text_len, plugin_layout_render_group_t group);

int plugin_layout_font_meta_font_load_element_stroke(plugin_layout_module_t module, plugin_layout_font_element_t element);
int plugin_layout_font_meta_font_load_element_normal(plugin_layout_module_t module, plugin_layout_font_element_t element);

/*utils*/
int plugin_layout_font_meta_font_spans_add(
    plugin_layout_module_t module, plugin_layout_font_meta_font_t meta_font, int16_t x, int16_t y, int16_t width, uint8_t coverage);
    
#ifdef __cplusplus
}
#endif

#endif 
