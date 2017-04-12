#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_ucs4.h"
#include "cpe/utils/string_utils.h"
#include "render/cache/ui_cache_res.h"
#include "render/cache/ui_cache_color.h"
#include "plugin/layout/plugin_layout_render_group.h"
#include "plugin/layout/plugin_layout_utils.h"
#include "plugin_layout_layout_rich_i.h"
#include "plugin_layout_layout_rich_block_i.h"
#include "plugin_layout_layout_meta_i.h"
#include "plugin_layout_render_i.h"
#include "plugin_layout_render_node_i.h"
#include "plugin_layout_font_face_i.h"

plugin_layout_font_id_t plugin_layout_layout_rich_default_font_id(plugin_layout_layout_rich_t rich) {
    return &rich->m_default_font_id;
}

void plugin_layout_layout_rich_set_default_font_id(plugin_layout_layout_rich_t rich, plugin_layout_font_id_t font_id) {
    if (font_id) {
        rich->m_default_font_id = *font_id;
        plugin_layout_layout_from_data(rich)->m_render->m_need_update = 1;
    }
    else {
        bzero(&rich->m_default_font_id, sizeof(rich->m_default_font_id));
    }
}

plugin_layout_font_draw_t plugin_layout_layout_rich_default_font_draw(plugin_layout_layout_rich_t rich) {
    return &rich->m_default_font_draw;
}

void plugin_layout_layout_rich_set_default_font_draw(plugin_layout_layout_rich_t rich, plugin_layout_font_draw_t font_draw) {
    if (font_draw) {
        rich->m_default_font_draw = *font_draw;
        plugin_layout_layout_from_data(rich)->m_render->m_need_update = 1;
    }
    else {
        bzero(&rich->m_default_font_draw, sizeof(rich->m_default_font_draw));
    }
}

uint8_t plugin_layout_layout_rich_line_break(plugin_layout_layout_rich_t rich) {
    return rich->m_line_break;
}

void plugin_layout_layout_rich_set_line_break(plugin_layout_layout_rich_t rich, uint8_t line_break) {
    line_break = line_break ? 1 : 0;
    
    if (rich->m_line_break != line_break) {
        rich->m_line_break = line_break;
        plugin_layout_layout_from_data(rich)->m_render->m_need_update = 1;
    }
}

void plugin_layout_layout_rich_clear_blocks(plugin_layout_layout_rich_t rich) {
    while(!TAILQ_EMPTY(&rich->m_blocks)) {
        plugin_layout_layout_rich_block_free(TAILQ_FIRST(&rich->m_blocks));
    }
}

plugin_layout_align_t plugin_layout_layout_rich_align(plugin_layout_layout_rich_t rich) {
    return rich->m_align;
}

void plugin_layout_layout_rich_set_align(plugin_layout_layout_rich_t rich, plugin_layout_align_t align) {
    if (rich->m_align != align) {
        rich->m_align = align;
        plugin_layout_layout_from_data(rich)->m_render->m_need_update = 1;
    }
}

static int plugin_layout_layout_rich_init(plugin_layout_layout_t layout) {
    plugin_layout_module_t module = layout->m_render->m_module;
    plugin_layout_layout_rich_t rich = (void*)plugin_layout_layout_data(layout);

    bzero(rich, sizeof(*rich));
    rich->m_default_font_id = module->m_default_font_id;

    TAILQ_INIT(&rich->m_blocks);
    return 0;
}

static void plugin_layout_layout_rich_fini(plugin_layout_layout_t layout) {
    plugin_layout_layout_rich_t rich = (void*)plugin_layout_layout_data(layout);
    plugin_layout_layout_rich_clear_blocks(rich);
}

static int plugin_layout_layout_rich_setup(plugin_layout_layout_t layout, char * args) {
    return 0;
}

static int plugin_layout_layout_rich_add_block(
    plugin_layout_module_t module, plugin_layout_layout_rich_t rich,
    char * args, const char * context_begin, const char * context_end)
{
    plugin_layout_layout_rich_block_t block;

    assert(context_begin <= context_end);
    
    if (context_begin == context_end) return 0;

    block = plugin_layout_layout_rich_block_create(rich);
    if (block == NULL) return -1;
    if (plugin_layout_layout_rich_block_set_context_range(block, context_begin, context_end, 0) != 0) return -1;

    if (args) {
        const char * str_value;

        if ((str_value = cpe_str_read_and_remove_arg(args, "color", ',', ':'))) {
            ui_color c;
            ui_cache_get_color(module->m_cache_mgr, str_value, &UI_COLOR_WHITE, &c);
            plugin_layout_layout_rich_block_set_color(block, &c);
        }

        if ((str_value = cpe_str_read_and_remove_arg(args, "size", ',', ':'))) {
            if (str_value[0] == '+' || str_value[0] == '-') {
                plugin_layout_layout_rich_block_set_adj_size(block, atoi(str_value));
            }
            else {
                plugin_layout_layout_rich_block_set_size(block, atoi(str_value));
            }
        }
    }
    
    return 0;
}

static int plugin_layout_layout_rich_analize(plugin_layout_layout_t layout) {
    plugin_layout_module_t module = layout->m_render->m_module;
    plugin_layout_layout_rich_t rich = (plugin_layout_layout_rich_t)plugin_layout_layout_data(layout);
    plugin_layout_render_t render = layout->m_render;
    const char * msg = render->m_data;
    const char * arg_begin;
    const char * arg_end;
    int rv = 0;
    
    plugin_layout_layout_rich_clear_blocks(rich);
    if (msg == NULL) return 0;

    arg_begin = strstr(msg, "'/");
    if (arg_begin == NULL) {
        return plugin_layout_layout_rich_add_block(module, rich, NULL, msg, msg + strlen(msg));
    }
    else {
        if (plugin_layout_layout_rich_add_block(module, rich, NULL, msg, arg_begin) != 0) rv = -1;
        msg = arg_begin;
    }
    
    while((arg_end = strstr(msg, "/'"))) {
        char arg_buf[64];
        const char * next_arg_begin;

        assert(cpe_str_start_with(msg, "'/"));
        msg += 2;

        cpe_str_dup_range(arg_buf, sizeof(arg_buf), msg, arg_end);
        msg = arg_end + 2;

        next_arg_begin = strstr(msg, "'/");
        if (next_arg_begin) {
            if (plugin_layout_layout_rich_add_block(module, rich, arg_buf, msg, next_arg_begin) != 0) rv = -1;
            msg = next_arg_begin;
        }
        else {
            size_t msg_len = strlen(msg);
            if (plugin_layout_layout_rich_add_block(module, rich, arg_buf, msg, msg + msg_len) != 0) rv = -1;
            msg += msg_len;
        }
    }

    if (msg[0] != 0) rv = -1;

    return rv;
}

static int plugin_layout_layout_rich_layout_commit_line(
    plugin_layout_module_t module, plugin_layout_render_group_t line_group, ui_rect_t r_line_rect)
{
    ui_rect line_rect;
    struct plugin_layout_render_node_it node_it;
    plugin_layout_render_node_t node;
    ui_vector_2 adj_pt;

    /*计算当前行包围框 */
    plugin_layout_render_group_bound_rt(line_group, &line_rect);

    /*所有单位底部对齐 */
    plugin_layout_render_group_nodes(line_group, &node_it);
    while((node = plugin_layout_render_node_it_next(&node_it))) {
        ui_rect_t node_bound = plugin_layout_render_node_bound_rt(node);

        if (node_bound->rb.y != line_rect.rb.y) {
            adj_pt.x = 0.0f;
            adj_pt.y = line_rect.rb.y - node_bound->rb.y;
            plugin_layout_render_node_adj_pos(node, &adj_pt);
        }

        //printf("xxxxx:               node=(%f,%f)-(%f,%f)\n", node_bound->lt.x, node_bound->lt.y, node_bound->rb.x, node_bound->rb.y);
    }

    /*清理当前行数据 */
    plugin_layout_render_group_clear(line_group);
    
    if (r_line_rect) *r_line_rect = line_rect;
    return 0;
}

static int plugin_layout_layout_rich_layout_block(
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

            /*当前行数据整理排布 */
            if (plugin_layout_layout_rich_layout_commit_line(module, line_group, last_line_rect) != 0) {
                rv = -1;
            }
    
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

static int plugin_layout_layout_rich_layout(plugin_layout_layout_t layout) {
    plugin_layout_module_t module = layout->m_render->m_module;
    plugin_layout_layout_rich_t rich = (plugin_layout_layout_rich_t)plugin_layout_layout_data(layout);
    plugin_layout_render_t render = layout->m_render;
    plugin_layout_layout_rich_block_t block;
    ui_vector_2 pt = UI_VECTOR_2_ZERO;
    ui_rect last_line_rect = UI_RECT_ZERO;
    ui_rect target_rect = UI_RECT_INITLIZER( 0.0f, 0.0f, render->m_size.x, render->m_size.y );
    plugin_layout_render_group_t group;
    plugin_layout_render_node_t last_placed_node;
    uint32_t sep_char = (uint32_t)L'\n';
    uint32_t group_count_origin = module->m_group_count;
    int rv = 0;
    
    if (render->m_data == NULL) return -1;

    group = plugin_layout_render_group_create(render);
    if (group == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_layout_basi_layout: create group fail!");
        return -1;
    }

    TAILQ_FOREACH(block, &rich->m_blocks, m_next) {
        cpe_str_ucs4_t text;
        size_t text_len;
        uint32_t const * text_data;
        plugin_layout_font_face_t face;
        size_t sep_pos, used_len;

        if (block->m_text_begin == block->m_text_end) continue;
        
        face = plugin_layout_font_face_check_create(module, block->m_font_id);
        if (face == NULL) { rv = -1; continue; }

        text = cpe_str_ucs4_from_utf8_range(module->m_alloc, block->m_text_begin, block->m_text_end);
        if (text == NULL) {
            CPE_ERROR(module->m_em, "plugin_layout_layout_rich_layout: convert context to ucs4 fail!");
            rv = -1;
            continue;
        }

        text_len = cpe_str_ucs4_len(text);
        text_data = cpe_str_ucs4_data(text);

        for(sep_pos = 0, used_len = 0; sep_pos < text_len; ++sep_pos) {
            if (text_data[sep_pos] == sep_char) { /*发现强制换行 */
                if ((sep_pos - used_len) == 0 && plugin_layout_render_group_node_count(group) == 0) { /* 空行 */
                    float last_line_height = ui_rect_height(&last_line_rect);
                    if (last_line_height > 0.0f) { /*不是第一行 */
                        last_line_rect.lt.x = target_rect.lt.x;
                        last_line_rect.lt.y = last_line_rect.rb.y + block->m_font_draw->grap_vert;
                        
                        last_line_rect.rb.y = last_line_rect.lt.y + last_line_height;
                        last_line_rect.rb.x = target_rect.lt.x;
                    }
                }
                else {
                    if (plugin_layout_layout_rich_layout_block(
                            module, render,
                            text_data + used_len, sep_pos - used_len,
                            face, block->m_font_draw, &target_rect,
                            group, &pt, &last_line_rect) != 0)
                    {
                        rv = -1;
                    }
                }
                used_len = sep_pos + 1;
                
                /*由于当前强制换行，如果有没有放置完成的行，则直接结束这个行 */
                if (plugin_layout_render_group_node_count(group) > 0) {
                    if (plugin_layout_layout_rich_layout_commit_line(module, group, &last_line_rect) != 0) {
                        rv = -1;
                    }
                }

                pt.x = target_rect.lt.x;
                pt.y = last_line_rect.rb.y + rich->m_default_font_draw.grap_vert;
            }
        }

        /*将当前block的剩余数据放置进去 */
        if (plugin_layout_layout_rich_layout_block(
                module, render,
                text_data + used_len, text_len - used_len,
                face, block->m_font_draw, &target_rect,
                group, &pt, &last_line_rect) != 0)
        {
            rv = -1;
        }

        /*移动放置点 */
        if ((last_placed_node = plugin_layout_render_group_last_node(group))) {
            /*如果最后行还有数据，则放置在这些数据之后 */
            ui_rect_t last_node_bound = plugin_layout_render_node_bound_rt(last_placed_node);
            
            pt.x = last_node_bound->rb.x;
            pt.y = last_node_bound->lt.y;
        }
        else {
            /*没有未放置数据，则放置在下一行 */
            pt.x = target_rect.lt.x;
            pt.y = last_line_rect.rb.y + block->m_font_draw->grap_vert;
        }
        
        cpe_str_ucs4_free(text);
    }

    /*全部block放置完成后，还有没有放置的行，则直接结束这一行 */
    if (plugin_layout_render_group_node_count(group) > 0) {
        plugin_layout_layout_rich_layout_commit_line(module, group, NULL);
    }
    
    plugin_layout_render_group_free(group);
    
    if (plugin_layout_align_in_rect(render, &target_rect, rich->m_align) != 0) rv = -1;

    assert(group_count_origin == module->m_group_count);
    
    return rv;
}

int plugin_layout_layout_rich_register(plugin_layout_module_t module) {
    plugin_layout_layout_meta_t meta;

    meta = 
        plugin_layout_layout_meta_create(
            module, "rich", sizeof(struct plugin_layout_layout_rich),
            plugin_layout_layout_rich_init,
            plugin_layout_layout_rich_fini,
            plugin_layout_layout_rich_setup,
            plugin_layout_layout_rich_analize,
            plugin_layout_layout_rich_layout,
            NULL);
    if (meta == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_layout_rich_register: name duplicate");
        return -1;
    }
    
    return 0;
}

void plugin_layout_layout_rich_unregister(plugin_layout_module_t module) {
    plugin_layout_layout_meta_t meta;

    meta = plugin_layout_layout_meta_find(module, "rich");
    if (meta) {
        plugin_layout_layout_meta_free(meta);
    }
}
