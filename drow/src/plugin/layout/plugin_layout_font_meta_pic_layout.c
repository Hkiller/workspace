#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_rect.h"
#include "plugin/layout/plugin_layout_render_group.h"
#include "plugin_layout_font_meta_pic_i.h"
#include "plugin_layout_font_face_i.h"
#include "plugin_layout_font_cache_i.h"
#include "plugin_layout_font_element_i.h"
#include "plugin_layout_render_i.h"
#include "plugin_layout_render_node_i.h"

int plugin_layout_font_meta_pic_basic_layout(
    void * ctx, plugin_layout_font_face_t face, plugin_layout_render_t render, plugin_layout_font_draw_t font_draw,
    uint32_t const * text, size_t text_len, plugin_layout_render_group_t group)
{
    ui_vector_2 pt = UI_VECTOR_2_INITLIZER( 0.0f, 0.0f );
    size_t i;

    for(i = 0; i < text_len; ++i) {
        plugin_layout_font_element_t font_element;
        plugin_layout_render_node_t node;

        font_element = plugin_layout_font_element_check_create(face, text[i]);
        if (font_element == NULL) continue;

        node = plugin_layout_render_node_create(render, font_element, font_draw);
        node->m_render_rt.lt = pt;
        node->m_render_rt.rb.x = node->m_render_rt.lt.x + font_element->m_render_width;
        node->m_render_rt.rb.y = node->m_render_rt.lt.y + font_element->m_render_height;

        node->m_bound_rt = node->m_render_rt;
        
        pt.x += font_element->m_render_width;

        if (group) plugin_layout_render_group_add_node(group, node);
    }
    
    return 0;
}

