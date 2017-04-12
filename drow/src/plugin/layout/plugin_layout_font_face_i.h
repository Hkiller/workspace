#ifndef PLUGIN_SPINE_FONT_SYS_FACE_I_H
#define PLUGIN_SPINE_FONT_SYS_FACE_I_H
#include "plugin/layout/plugin_layout_font_face.h"
#include "plugin/layout/plugin_layout_font_info.h"
#include "plugin_layout_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_layout_font_face {
    plugin_layout_module_t m_module;
    cpe_hash_entry m_hh;
    plugin_layout_font_meta_t m_meta;
    TAILQ_ENTRY(plugin_layout_font_face) m_next_for_meta;
    struct plugin_layout_font_id m_id;
    uint16_t m_ascender;
    uint16_t m_height;
    struct cpe_hash_table m_elements;
};

int plugin_layout_font_face_basic_layout(
    plugin_layout_font_face_t face, plugin_layout_render_t render,
    plugin_layout_font_draw_t font_draw, uint32_t const * text, size_t text_len,
    plugin_layout_render_group_t group);
    
uint32_t plugin_layout_font_face_hash(const plugin_layout_font_face_t face);
int plugin_layout_font_face_eq(const plugin_layout_font_face_t l, const plugin_layout_font_face_t r);
    
#ifdef __cplusplus
}
#endif

#endif
