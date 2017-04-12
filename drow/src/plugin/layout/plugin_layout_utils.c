#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_ucs4.h"
#include "render/utils/ui_rect.h"
#include "plugin/layout/plugin_layout_render_group.h"
#include "plugin_layout_utils_i.h"
#include "plugin_layout_font_face_i.h"
#include "plugin_layout_render_i.h"
#include "plugin_layout_render_node_i.h"

int plugin_layout_align_in_rect(
    plugin_layout_render_t render,
    ui_rect_t target_rect, plugin_layout_align_t align)
{
    plugin_layout_render_group_t group;
    plugin_layout_render_node_t node;
    ui_rect group_rt;
    ui_vector_2 adj_pt;

    group = plugin_layout_render_group_create(render);
    if (group == NULL) {
        CPE_ERROR(render->m_module->m_em, "plugin_layout_align_in_rect: create group fail!");
        return -1;
    }

    /*水平方向对每行处理 */
    for(node = TAILQ_FIRST(&render->m_nodes); node; node = TAILQ_NEXT(node, m_next)) {
        plugin_layout_render_group_clear(group);
        if (plugin_layout_select_line(node, group) != 0) continue;

        adj_pt.y = 0.0f;
        plugin_layout_render_group_bound_rt(group, &group_rt);
        switch(align) {
        case plugin_layout_align_left_top:
        case plugin_layout_align_left_center:
        case plugin_layout_align_left_bottom:
            adj_pt.x = target_rect->lt.x - group_rt.lt.x;
            break;
        case plugin_layout_align_center_top:
        case plugin_layout_align_center_center:
        case plugin_layout_align_center_bottom:
            adj_pt.x = (ui_rect_width(target_rect) - ui_rect_width(&group_rt)) / 2.0f;
            break;
        case plugin_layout_align_right_top:
        case plugin_layout_align_right_center:
        case plugin_layout_align_right_bottom:
            adj_pt.x = target_rect->rb.x - group_rt.rb.x;
            break;
        }
        plugin_layout_render_group_adj_pos(group, &adj_pt);
            
        node = plugin_layout_render_group_last_node(group);
        assert(node);
    }

    /*垂直方向整体排布 */
    adj_pt.x = 0.0f;
    plugin_layout_render_bound_rt(render, &group_rt);
    switch(align) {
    case plugin_layout_align_left_top:
    case plugin_layout_align_center_top:
    case plugin_layout_align_right_top:
        adj_pt.y = target_rect->lt.y - group_rt.lt.y;
        break;
    case plugin_layout_align_center_center:
    case plugin_layout_align_left_center:
    case plugin_layout_align_right_center:
        adj_pt.y = (ui_rect_height(target_rect) - ui_rect_height(&group_rt)) / 2.0f;
        break;
    case plugin_layout_align_right_bottom:
    case plugin_layout_align_left_bottom:
    case plugin_layout_align_center_bottom:
        adj_pt.y = target_rect->rb.y - group_rt.rb.y;
        break;
    }
    plugin_layout_render_adj_pos(render, &adj_pt);

    plugin_layout_render_group_free(group);
    return 0;
}

int plugin_layout_select_line(
    plugin_layout_render_node_t start_node, plugin_layout_render_group_t group)
{
    ui_rect_t base_rect = plugin_layout_render_node_bound_rt(start_node);

    if (plugin_layout_render_group_add_node(group, start_node) != 0) {
        CPE_ERROR(start_node->m_render->m_module->m_em, "plugin_layout_select_line: add to result group fail!");
        return -1;
    }

    for(start_node = TAILQ_NEXT(start_node, m_next); start_node; start_node = TAILQ_NEXT(start_node, m_next)) {
        ui_rect_t check_rect = plugin_layout_render_node_bound_rt(start_node);

        if (check_rect->lt.y >= base_rect->rb.y || check_rect->rb.y <= base_rect->lt.y) break;

        if (plugin_layout_render_group_add_node(group, start_node) != 0) {
            CPE_ERROR(start_node->m_render->m_module->m_em, "plugin_layout_select_line: add to result group fail!");
            return -1;
        }
    } 
    
    return 0;
}

size_t plugin_layout_render_node_wlen(plugin_layout_render_node_it_t node_it) {
    size_t len = 0;
    plugin_layout_render_node_t node;

    while((node = plugin_layout_render_node_it_next(node_it))) {
        len++;
    }

    return len;
}

cpe_str_ucs4_t plugin_layout_render_nodes_to_ucs4(mem_allocrator_t alloc, plugin_layout_render_node_it_t node_it) {
    cpe_str_ucs4_t r;
    size_t count;
    plugin_layout_render_node_t node;
    struct plugin_layout_render_node_it tmp_it;
    uint32_t * buf;
    uint32_t i;

    tmp_it = *node_it;
    count = plugin_layout_render_node_wlen(&tmp_it);

    r = cpe_str_ucs4_alloc(alloc, count);
    if (r == NULL) return NULL;
    buf = cpe_str_ucs4_data(r);

    i = 0;
    while((node = plugin_layout_render_node_it_next(node_it))) {
        uint32_t c = plugin_layout_render_node_charter(node);
        if (c) buf[i++] = c;
    }

    return r;
}

size_t plugin_layout_render_node_clen(plugin_layout_render_node_it_t node_it) {
    size_t r_len = 0;
    plugin_layout_render_node_t node;

    while((node = plugin_layout_render_node_it_next(node_it))) {
        uint32_t c = plugin_layout_render_node_charter(node);
        r_len += cpe_char_ucs4_clen(c);
    }

    return r_len;
}

char * plugin_layout_render_nodes_to_utf8(mem_allocrator_t alloc, plugin_layout_render_node_it_t node_it) {
    char * r;
    size_t count;
    plugin_layout_render_node_t node;
    struct plugin_layout_render_node_it tmp_it;
    size_t used_len;
    
    tmp_it = *node_it;
    count = plugin_layout_render_node_wlen(&tmp_it);

    r = mem_alloc(alloc, count + 1);
    if (r == NULL) return NULL;

    used_len = 0;
    while((node = plugin_layout_render_node_it_next(node_it))) {
        uint32_t c = plugin_layout_render_node_charter(node);
        used_len += cpe_char_ucs4_to_utf8(r + used_len, count - used_len, c);
	}

    r[used_len] = 0;
    
    return r;
}
