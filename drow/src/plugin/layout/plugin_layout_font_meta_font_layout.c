#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_rect.h"
#include "plugin/layout/plugin_layout_render_group.h"
#include "plugin_layout_font_meta_font_i.h"
#include "plugin_layout_font_face_i.h"
#include "plugin_layout_font_cache_i.h"
#include "plugin_layout_font_element_i.h"
#include "plugin_layout_render_i.h"
#include "plugin_layout_render_node_i.h"

int plugin_layout_font_meta_font_basic_layout(
    void * ctx, plugin_layout_font_face_t face, plugin_layout_render_t render, plugin_layout_font_draw_t font_draw,
    uint32_t const * text, size_t text_len, plugin_layout_render_group_t group)
{
    plugin_layout_font_face_font_t face_font = plugin_layout_font_face_data(face);
    hb_buffer_t *buffer;
    hb_glyph_info_t *glyphs;
    unsigned int glyph_count;
    unsigned int glyph_i;
    hb_glyph_position_t * glyph_poses;
    ui_vector_2 pt;
    plugin_layout_render_node_t pre_node;
    
    buffer = hb_buffer_create();

    hb_buffer_set_direction(buffer, HB_DIRECTION_LTR);
    //hb_buffer_set_script(buffer, text.script);
    //hb_buffer_set_language(buffer, hb_language_from_string(text.language.c_str(), text.language.size()));

    hb_buffer_add_utf32(buffer, text, text_len, 0, text_len);

    //hb_shape(font, buffer, features.empty() ? NULL : &features[0], features.size());
    hb_shape(face_font->m_hb_font, buffer, NULL, 0); /*use features ?*/

    glyphs = hb_buffer_get_glyph_infos(buffer, &glyph_count);
    glyph_poses = hb_buffer_get_glyph_positions(buffer, &glyph_count);

    pt.x = 0.0f;
    pt.y = 0.0f;

    pre_node = NULL;
    for(glyph_i = 0; glyph_i < glyph_count; ++glyph_i) {
        hb_glyph_info_t * glyph = glyphs + glyph_i;
        hb_glyph_position_t * glyph_pos = glyph_poses + glyph_i;
        hb_position_t kernX = 0;
        hb_position_t kernY = 0;
        plugin_layout_font_element_t font_element;
        plugin_layout_render_node_t node;
        float kx, ky, xa, ya, xo, yo;
        
        if(glyph_i > 0) {
            hb_font_get_glyph_kerning_for_direction(
                face_font->m_hb_font,
                glyphs[glyph_i - 1].codepoint,
                glyph->codepoint,
                HB_DIRECTION_LTR,
                &kernX, &kernY);
        }

        kx = (float) kernX / 64;
        ky = (float) kernY / 64;
        xa = (float) glyph_pos->x_advance / 64 + kx;
        ya = (float) glyph_pos->y_advance / 64 + ky;
        xo = (float) glyph_pos->x_offset / 64;
        yo = (float) glyph_pos->y_offset / 64;

        font_element = plugin_layout_font_element_check_create(face, text[glyph_i]);
        if (font_element) {
            node = plugin_layout_render_node_create(render, font_element, font_draw);
        
            node->m_render_rt.lt.x = pt.x + font_element->m_bearing_x + xo;
            node->m_render_rt.lt.y = pt.y - font_element->m_bearing_y + face->m_ascender + yo;
            node->m_render_rt.rb.x = node->m_render_rt.lt.x + font_element->m_render_width;
            node->m_render_rt.rb.y = node->m_render_rt.lt.y + font_element->m_render_height;
        }
        else {
            node = plugin_layout_render_node_create(render, NULL, NULL);
        }
        
        node->m_bound_rt.lt = pt;
        node->m_bound_rt.rb.x = pt.x + xa;
        node->m_bound_rt.rb.y = pt.y + face->m_height;
        
        /* printf( */
        /*     "xxxxx: create node (%f,%f)-(%f,%f) bound (%f,%f)-(%f,%f)\n", */
        /*     node->m_render_rt.lt.x, node->m_render_rt.lt.y, node->m_render_rt.rb.x, node->m_render_rt.rb.y, */
        /*     node->m_bound_rt.lt.x, node->m_bound_rt.lt.y, node->m_bound_rt.rb.x, node->m_bound_rt.rb.y); */

        pt.x += xa + face->m_id.stroke_width * 1u;
        pt.y += ya;

        if (pre_node) {
            pre_node->m_bound_rt.rb.x -= kx;
            pre_node->m_bound_rt.rb.y -= ky;
        }
        
        pre_node = node;
        if (group ) plugin_layout_render_group_add_node(group, node);
    }

    hb_buffer_destroy(buffer);
    
    return 0;
}

