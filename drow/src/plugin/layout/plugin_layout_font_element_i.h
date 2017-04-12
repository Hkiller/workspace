#ifndef PLUGIN_SPINE_FONT_ELEMENT_I_H
#define PLUGIN_SPINE_FONT_ELEMENT_I_H
#include "render/utils/ui_rect.h"
#include "plugin/layout/plugin_layout_font_element.h"
#include "plugin_layout_font_face_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_layout_font_element {
    plugin_layout_font_face_t m_face;
    TAILQ_ENTRY(plugin_layout_font_element) m_next;
    struct cpe_hash_entry m_hh;
    uint32_t m_charter;
    uint16_t m_node_count;

    ui_rect m_texture_rect;

    uint16_t m_render_width;
    uint16_t m_render_height;
    int16_t m_bearing_x;
    int16_t	m_bearing_y;
};

void plugin_layout_font_element_free_all(plugin_layout_font_face_t face);
void plugin_layout_font_element_real_free(plugin_layout_font_element_t element);    
    
uint32_t plugin_layout_font_element_hash(const plugin_layout_font_element_t element);
int plugin_layout_font_element_eq(const plugin_layout_font_element_t l, const plugin_layout_font_element_t r);
    
#ifdef __cplusplus
}
#endif

#endif
