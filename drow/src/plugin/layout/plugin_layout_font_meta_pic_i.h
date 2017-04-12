#ifndef PLUGIN_LAYOUT_FONT_META_ART_I_H
#define PLUGIN_LAYOUT_FONT_META_ART_I_H
#include "render/model/ui_data_module.h"
#include "plugin/layout/plugin_layout_font_meta_pic.h"
#include "plugin_layout_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_layout_font_face_pic {
    ui_data_src_t m_module_src;
    ui_data_module_t m_module;
};

int plugin_layout_font_meta_pic_register(plugin_layout_module_t module); 
void plugin_layout_font_meta_pic_unregister(plugin_layout_module_t module);

int plugin_layout_font_meta_pic_basic_layout(
    void * ctx, plugin_layout_font_face_t face,
    plugin_layout_render_t render, plugin_layout_font_draw_t font_draw,
    uint32_t const * text, size_t text_len, plugin_layout_render_group_t group);
    
#ifdef __cplusplus
}
#endif

#endif 
