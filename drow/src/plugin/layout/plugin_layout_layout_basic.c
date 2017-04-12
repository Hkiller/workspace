#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_ucs4.h"
#include "render/cache/ui_cache_res.h"
#include "plugin/layout/plugin_layout_render_group.h"
#include "plugin/layout/plugin_layout_utils.h"
#include "plugin_layout_layout_basic_i.h"
#include "plugin_layout_layout_meta_i.h"
#include "plugin_layout_render_i.h"
#include "plugin_layout_render_node_i.h"
#include "plugin_layout_font_face_i.h"

plugin_layout_font_id_t plugin_layout_layout_basic_font_id(plugin_layout_layout_basic_t basic) {
    return &basic->m_font_id;
}

void plugin_layout_layout_basic_set_font_id(plugin_layout_layout_basic_t basic, plugin_layout_font_id_t font_id) {
    if (font_id) {
        basic->m_font_id = *font_id;
        plugin_layout_layout_from_data(basic)->m_render->m_need_update = 1;
    }
    else {
        bzero(&basic->m_font_id, sizeof(basic->m_font_id));
    }
}

plugin_layout_font_draw_t plugin_layout_layout_basic_font_draw(plugin_layout_layout_basic_t basic) {
    return &basic->m_font_draw;
}

void plugin_layout_layout_basic_set_font_draw(plugin_layout_layout_basic_t basic, plugin_layout_font_draw_t font_draw) {
    if (font_draw) {
        if (basic->m_font_draw.grap_horz != font_draw->grap_horz
            || basic->m_font_draw.grap_vert != font_draw->grap_vert)
        {
            plugin_layout_layout_from_data(basic)->m_render->m_need_update = 1;
        }

        basic->m_font_draw = *font_draw;
    }
    else {
        bzero(&basic->m_font_draw, sizeof(basic->m_font_draw));
    }
}

uint8_t plugin_layout_layout_basic_line_break(plugin_layout_layout_basic_t basic) {
    return basic->m_line_break;
}

void plugin_layout_layout_basic_set_line_break(plugin_layout_layout_basic_t basic, uint8_t line_break) {
    line_break = line_break ? 1 : 0;
    
    if (basic->m_line_break != line_break) {
        basic->m_line_break = line_break;
        plugin_layout_layout_from_data(basic)->m_render->m_need_update = 1;
    }
}

plugin_layout_align_t plugin_layout_layout_basic_align(plugin_layout_layout_basic_t basic) {
    return basic->m_align;
}

void plugin_layout_layout_basic_set_align(plugin_layout_layout_basic_t basic, plugin_layout_align_t align) {
    if (basic->m_align != align) {
        basic->m_align = align;
        plugin_layout_layout_from_data(basic)->m_render->m_need_update = 1;
    }
}

static int plugin_layout_layout_basic_init(plugin_layout_layout_t layout) {
    plugin_layout_module_t module = layout->m_render->m_module;
    plugin_layout_layout_basic_t basic = (void*)plugin_layout_layout_data(layout);

    bzero(basic, sizeof(*basic));
    basic->m_font_id = module->m_default_font_id;

    return 0;
}

static void plugin_layout_layout_basic_fini(plugin_layout_layout_t layout) {
}

static int plugin_layout_layout_basic_setup(plugin_layout_layout_t layout, char * args) {
    return 0;
}

static int plugin_layout_layout_basic_analize(plugin_layout_layout_t layout) {
    return 0;
}

static int plugin_layout_layout_basic_layout_block(
    plugin_layout_module_t module, plugin_layout_render_t render,
    uint32_t const * text_data, uint32_t text_len,
    plugin_layout_font_face_t face, plugin_layout_font_draw_t draw, ui_rect_t target_rect,
    plugin_layout_render_group_t line_group, ui_vector_2_t pt, ui_rect_t last_line_rect)
{
    plugin_layout_render_group_t block_group;
    plugin_layout_render_node_t node;
    int rv = 0;

    block_group = plugin_layout_render_group_create(render);
    if (block_group == NULL) {
        CPE_ERROR(render->m_module->m_em, "plugin_layout_layout_rich_layout_block: create group fail!");
        return -1;
    }

    /*首先连接当前内容，向后放置所有数据 */
    if (plugin_layout_font_face_basic_layout(face, render, draw, text_data, text_len, block_group) != 0) {
        plugin_layout_render_group_free(block_group);
        return -1;
    }
    plugin_layout_render_group_adj_pos(block_group, pt);

    /*然后进行分行处理 */
    while((node = plugin_layout_render_group_first_node(block_group))) {
        ui_rect_t node_bound = plugin_layout_render_node_bound_rt(node);

        if (node_bound->rb.x > target_rect->rb.x /*越界 */
            && cpe_float_cmp(node_bound->lt.x, target_rect->lt.x, UI_FLOAT_PRECISION) != 0 /*不是起始节点 */)
        {
            /*换行 */
            ui_vector_2 pos_adj;

            /*计算当前行包围框 */
            plugin_layout_render_group_bound_rt(line_group, last_line_rect);
            plugin_layout_render_group_clear(line_group);
    
            /*讲剩余内容放置到下一行 */
            pos_adj.x = target_rect->lt.x - node_bound->lt.x;
            pos_adj.y = ui_rect_height(last_line_rect) + draw->grap_vert;
            plugin_layout_render_group_adj_pos(block_group, &pos_adj);
        }

        /*将当前节点移动到line行上 */
        plugin_layout_render_group_add_node(line_group, node);
        plugin_layout_render_group_remove_node(block_group, node);
    }

    plugin_layout_render_group_free(block_group);
    
    return rv;
}


static int plugin_layout_layout_basic_layout(plugin_layout_layout_t layout) {
    plugin_layout_module_t module = layout->m_render->m_module;
    plugin_layout_layout_basic_t basic = (plugin_layout_layout_basic_t)plugin_layout_layout_data(layout);
    plugin_layout_render_t render = layout->m_render;
    plugin_layout_font_face_t face;
    cpe_str_ucs4_t text;
    size_t text_len;
    uint32_t const * text_data;
    ui_vector_2 pt = UI_VECTOR_2_ZERO;
    ui_rect last_line_rect = UI_RECT_ZERO;
    ui_rect target_rect = UI_RECT_INITLIZER( 0.0f, 0.0f, render->m_size.x, render->m_size.y );
    plugin_layout_render_group_t group;
    uint32_t group_count_origin = module->m_group_count;
    int rv = 0;

    if (render->m_data == NULL) return -1;
    
    face = plugin_layout_font_face_check_create(module, &basic->m_font_id);
    if (face == NULL) return -1;

    text = cpe_str_ucs4_from_utf8(module->m_alloc, render->m_data);
    if (text == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_layout_basi_layout: convert context to ucs4 fail!");
        return -1;
    }

    text_len = cpe_str_ucs4_len(text);
    text_data = cpe_str_ucs4_data(text);

    group = plugin_layout_render_group_create(render);
    if (group == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_layout_basi_layout: create group fail!");
        cpe_str_ucs4_free(text);
        return -1;
    }

    /*分行 */
	if (!basic->m_line_break) {
        if (plugin_layout_font_face_basic_layout(face, render, &basic->m_font_draw, text_data, text_len, group) != 0) {
            rv = -1;
        }
    }
    else {
        size_t sep_pos, used_len;
        uint32_t sep_char = (uint32_t)L'\n';
        
        for(sep_pos = 0, used_len = 0; sep_pos < text_len; ++sep_pos) {
            if (text_data[sep_pos] == sep_char) {
                if ((sep_pos - used_len) == 0 && plugin_layout_render_group_node_count(group) == 0) { /* 空行 */
                    float last_line_height = ui_rect_height(&last_line_rect);
                    if (last_line_height > 0.0f) { /*不是第一行 */
                        last_line_rect.lt.x = target_rect.lt.x;
                        last_line_rect.lt.y = last_line_rect.rb.y + basic->m_font_draw.grap_vert;
                        
                        last_line_rect.rb.y = last_line_rect.lt.y + last_line_height;
                        last_line_rect.rb.x = target_rect.lt.x;
                    }
                }
                else {
                    if (plugin_layout_layout_basic_layout_block(
                            module, render,
                            text_data + used_len, sep_pos - used_len,
                            face, &basic->m_font_draw, &target_rect,
                            group, &pt, &last_line_rect) != 0)
                    {
                        rv = -1;
                    }
                }
                used_len = sep_pos + 1;
                
                /*由于当前强制换行，如果有没有放置完成的行，则直接结束这个行 */
                if (plugin_layout_render_group_node_count(group) > 0) {
                    plugin_layout_render_group_bound_rt(group, &last_line_rect);
                    plugin_layout_render_group_clear(group);
                }

                pt.x = target_rect.lt.x;
                pt.y = last_line_rect.rb.y + basic->m_font_draw.grap_vert;
            }
        }

        if (plugin_layout_layout_basic_layout_block(
                module, render,
                text_data + used_len, sep_pos - used_len,
                face, &basic->m_font_draw, &target_rect,
                group, &pt, &last_line_rect) != 0)
        {
            rv = -1;
        }
    }

    plugin_layout_render_group_free(group);
    cpe_str_ucs4_free(text);

    if (plugin_layout_align_in_rect(render, &target_rect, basic->m_align) != 0) rv = -1;

    assert(group_count_origin == module->m_group_count);

    return rv;
}

static int plugin_layout_layout_basic_update(plugin_layout_layout_t layout, int begin_pos, int end_pos, uint32_t const * text, size_t text_len) {
    plugin_layout_render_t render = layout->m_render;
    plugin_layout_module_t module = render->m_module;
    cpe_str_ucs4_t old_ucs4;
    size_t new_len, old_len;
    uint32_t * old_data;
    uint32_t * new_data, *wp;
    size_t i;
    int rv;
    
    old_ucs4 = cpe_str_ucs4_from_utf8(module->m_alloc, plugin_layout_render_data(render));
    if (old_ucs4 == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_layout_basic_update: get old data fail!");
        return -1;
    }
    old_data = cpe_str_ucs4_data(old_ucs4);
    old_len = cpe_str_ucs4_len(old_ucs4);
    
    if (begin_pos < 0) begin_pos = 0;
    if (begin_pos > old_len) begin_pos = old_len;
    if (end_pos < 0 || end_pos > old_len) end_pos = old_len;

    new_len = begin_pos + (text_len) + (cpe_str_ucs4_len(old_ucs4) - end_pos);

    new_data = mem_alloc(module->m_alloc, sizeof(uint32_t) * new_len);
    if (new_data == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_layout_basic_update: alloc new data fail!");
        cpe_str_ucs4_free(old_ucs4);
        return -1;
    }
    wp = new_data;
    
    for(i = 0; i < begin_pos; ++i) {
        *wp++ = old_data[i];
    }

    for(i = 0; i < text_len; ++i) {
        *wp++ = text[i];
    }

    for(i = end_pos; i < old_len; ++i) {
        *wp++ = old_data[i];
    }

    rv = plugin_layout_render_set_data_ucs4(render, new_data, new_len);

    cpe_str_ucs4_free(old_ucs4);
    mem_free(module->m_alloc, new_data);

    return rv;
}

int plugin_layout_layout_basic_register(plugin_layout_module_t module) {
    plugin_layout_layout_meta_t meta;

    meta = 
        plugin_layout_layout_meta_create(
            module, "basic", sizeof(struct plugin_layout_layout_basic),
            plugin_layout_layout_basic_init,
            plugin_layout_layout_basic_fini,
            plugin_layout_layout_basic_setup,
            plugin_layout_layout_basic_analize,
            plugin_layout_layout_basic_layout,
            plugin_layout_layout_basic_update);
    if (meta == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_layout_basic_register: name duplicate");
        return -1;
    }
    
    return 0;
}

void plugin_layout_layout_basic_unregister(plugin_layout_module_t module) {
    plugin_layout_layout_meta_t meta;

    meta = plugin_layout_layout_meta_find(module, "basic");
    if (meta) {
        plugin_layout_layout_meta_free(meta);
    }
}
