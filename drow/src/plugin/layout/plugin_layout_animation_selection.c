#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_ucs4.h"
#include "render/utils/ui_color.h"
#include "render/cache/ui_cache_color.h"
#include "render/runtime/ui_runtime_render_utils.h"
#include "plugin/layout/plugin_layout_animation_meta.h"
#include "plugin/layout/plugin_layout_animation.h"
#include "plugin_layout_animation_selection_i.h"
#include "plugin_layout_animation_i.h"
#include "plugin_layout_render_i.h"
#include "plugin_layout_render_node_i.h"
#include "plugin_layout_layout_basic_i.h"
#include "plugin_layout_layout_rich_i.h"
#include "plugin_layout_font_face_i.h"

static void plugin_layout_animation_selection_layout(plugin_layout_animation_t animation, void * ctx);

plugin_layout_animation_selection_t
plugin_layout_animation_selection_create(plugin_layout_render_t render) {
    plugin_layout_animation_t animation;

    animation = plugin_layout_animation_create(render, render->m_module->m_animation_meta_selection);
    if (animation == NULL) return NULL;

    return plugin_layout_animation_data(animation);
}

void plugin_layout_animation_selection_free(plugin_layout_animation_selection_t selection) {
    plugin_layout_animation_t animation = plugin_layout_animation_from_data(selection);
    plugin_layout_animation_free(animation);
}

plugin_layout_animation_selection_t plugin_layout_animation_selection_from_animation(plugin_layout_animation_t animation) {
    return plugin_layout_animation_data(animation);
}

plugin_layout_animation_selection_t plugin_layout_animation_selection_find_first(plugin_layout_render_t render) {
    plugin_layout_animation_t animation;

    animation = plugin_layout_animation_find_first_by_type(render, "selection");

    return animation ? plugin_layout_animation_selection_from_animation(animation) : NULL;
}

plugin_layout_animation_selection_type_t plugin_layout_animation_selection_type(plugin_layout_animation_selection_t selection) {
    return selection->m_type;
}

void plugin_layout_animation_selection_set_type(plugin_layout_animation_selection_t selection, plugin_layout_animation_selection_type_t type) {
    if (type != selection->m_type) {
        selection->m_type = type;
    }
}

int plugin_layout_animation_selection_begin_pos(plugin_layout_animation_selection_t selection) {
    plugin_layout_animation_t animation;

    if (selection->m_type != plugin_layout_animation_selection_line) return -1;
    
    animation = plugin_layout_animation_from_data(selection);
    plugin_layout_render_do_layout(animation->m_render);
    return selection->m_begin_pos;
}

int plugin_layout_animation_selection_end_pos(plugin_layout_animation_selection_t selection) {
    plugin_layout_animation_t animation;

    if (selection->m_type != plugin_layout_animation_selection_line) return -1;
    
    animation = plugin_layout_animation_from_data(selection);
    plugin_layout_render_do_layout(animation->m_render);
    return selection->m_end_pos;
}

uint32_t plugin_layout_animation_selection_length(plugin_layout_animation_selection_t selection) {
    plugin_layout_animation_t animation;

    if (selection->m_type != plugin_layout_animation_selection_line) return 0;
    
    animation = plugin_layout_animation_from_data(selection);
    plugin_layout_render_do_layout(animation->m_render);

    return selection->m_end_pos > selection->m_begin_pos
        ? (selection->m_end_pos - selection->m_begin_pos)
        : 0;
}

void plugin_layout_animation_selection_set_range(plugin_layout_animation_selection_t selection, int begin, int end) {
    if (selection->m_type == plugin_layout_animation_selection_line) {
        selection->m_begin_pos = begin;
        selection->m_end_pos = end;
        selection->m_begin_pt = UI_VECTOR_2_ZERO;
        selection->m_end_pt = UI_VECTOR_2_ZERO;
    }
}

void plugin_layout_animation_selection_set_range_by_pt(plugin_layout_animation_selection_t selection, ui_vector_2_t begin_pt, ui_vector_2_t end_pt) {
    plugin_layout_animation_t animation = plugin_layout_animation_from_data(selection);

    selection->m_begin_pt = *begin_pt;
    selection->m_end_pt = *end_pt;
    selection->m_begin_pos = -1;
    selection->m_end_pos = -1;

    if (selection->m_type == plugin_layout_animation_selection_line && !animation->m_render->m_need_update) {
        plugin_layout_animation_selection_layout(animation, animation->m_render->m_module);        
    }
}

static void plugin_layout_animation_selection_layout(plugin_layout_animation_t animation, void * ctx) {
    plugin_layout_render_t render = animation->m_render;
    plugin_layout_animation_selection_t selection;
    
    selection = plugin_layout_animation_data(animation);

    if (selection->m_type == plugin_layout_animation_selection_line) { /*按行选择，将范围转换为字符范围 */
        plugin_layout_render_node_t node;
        ui_rect select_rect;
        int pos;

        if (selection->m_begin_pt.x == selection->m_end_pt.x && selection->m_begin_pt.y == selection->m_end_pt.y) return;

        select_rect.lt = selection->m_begin_pt;
        select_rect.rb = selection->m_end_pt;

        if (select_rect.lt.x > select_rect.rb.x) { float t = select_rect.lt.x; select_rect.lt.x = select_rect.rb.x; select_rect.rb.x = t; }
        if (select_rect.lt.y > select_rect.rb.y) { float t = select_rect.lt.y; select_rect.lt.y = select_rect.rb.y; select_rect.rb.y = t; }

        selection->m_begin_pos = -1;
        selection->m_end_pos = -1;

        pos = 0;
        TAILQ_FOREACH(node, &render->m_nodes, m_next) {
            if (ui_rect_is_intersection_valid(&node->m_bound_rt, &select_rect)) {
                if (selection->m_begin_pos == -1) {
                    selection->m_begin_pos = pos;
                }
                selection->m_end_pos = pos + 1;
            }
            pos++;
        }
    }
}

static int plugin_layout_animation_selection_init(plugin_layout_animation_t animation, void * ctx) {
    plugin_layout_module_t module = ctx;
    plugin_layout_animation_selection_t selection;
    
    selection = plugin_layout_animation_data(animation);
    
    if (ui_cache_find_color(module->m_cache_mgr, "selection", &selection->m_color) != 0) {
        if (ui_cache_find_color(module->m_cache_mgr, "blue", &selection->m_color) != 0) {
            CPE_ERROR(module->m_em, "plugin_layout_animation_selection_init: no selection color 'selection' or 'black'");
            return -1;
        }
    }

    selection->m_type = plugin_layout_animation_selection_line;
    selection->m_begin_pt = UI_VECTOR_2_ZERO;
    selection->m_end_pt = UI_VECTOR_2_ZERO;
    selection->m_begin_pos = -1;
    selection->m_end_pos = -1;
    
    return 0;
}

cpe_str_ucs4_t plugin_layout_animation_selection_text_ucs4(mem_allocrator_t alloc, plugin_layout_animation_selection_t selection) {
    plugin_layout_render_t render = plugin_layout_animation_from_data(selection)->m_render;
    plugin_layout_module_t module = render->m_module;
    
    if (selection->m_type == plugin_layout_animation_selection_line) {
        size_t count;
        int pos;
        cpe_str_ucs4_t r;
        uint32_t * wp;
        plugin_layout_render_node_t node;
        
        count = 0;
        pos = 0;
        TAILQ_FOREACH(node, &render->m_nodes, m_next) {
            if (pos >= selection->m_begin_pos && pos < selection->m_end_pos) count++;
            pos++;
        }

        r = mem_alloc(alloc, count + 1);
        if (r == NULL) return NULL;
        wp = cpe_str_ucs4_data(r);
        
        TAILQ_FOREACH(node, &render->m_nodes, m_next) {
            if (pos >= selection->m_begin_pos && pos < selection->m_end_pos) {
                *wp++ = plugin_layout_render_node_charter(node);
            }
        }

        return r;
    }
    else {
        CPE_ERROR(module->m_em, "plugin_layout_animation_selection_text_ucs4: not support block mod yet!");
        return NULL;
    }
}

char * plugin_layout_animation_selection_text_utf8(mem_allocrator_t alloc, plugin_layout_animation_selection_t selection) {
    plugin_layout_render_t render = plugin_layout_animation_from_data(selection)->m_render;
    plugin_layout_module_t module = render->m_module;
    
    if (selection->m_type == plugin_layout_animation_selection_line) {
        size_t count;
        int pos;
        char * r;
        size_t used_len;
        plugin_layout_render_node_t node;
        
        count = 0;
        pos = 0;
        TAILQ_FOREACH(node, &render->m_nodes, m_next) {
            if (pos >= selection->m_begin_pos && pos < selection->m_end_pos) {
                uint32_t c = plugin_layout_render_node_charter(node);
                count += cpe_char_ucs4_clen(c);
            }
            pos++;
        }

        r = mem_alloc(alloc, count + 1);
        if (r == NULL) return NULL;

        used_len = 0;
        TAILQ_FOREACH(node, &render->m_nodes, m_next) {
            if (pos >= selection->m_begin_pos && pos < selection->m_end_pos) {
                uint32_t c = plugin_layout_render_node_charter(node);
                used_len += cpe_char_ucs4_to_utf8(r + used_len, count - used_len, c);
            }
        }

        r[used_len] = 0;
        
        return r;
    }
    else {
        CPE_ERROR(module->m_em, "plugin_layout_animation_selection_text_utf8: not support block mod yet!");
        return NULL;
    }
}

int plugin_layout_animation_selection_set_text_utf8(plugin_layout_animation_selection_t selection, const char * utf8) {
    plugin_layout_render_t render = plugin_layout_animation_from_data(selection)->m_render;
    plugin_layout_module_t module = render->m_module;

    plugin_layout_render_do_layout(render);
    
    if (selection->m_type == plugin_layout_animation_selection_line) {
        return plugin_layout_render_update_text_utf8(render, selection->m_begin_pos, selection->m_end_pos, utf8);
    }
    else {
        CPE_ERROR(module->m_em, "plugin_layout_animation_selection_set_text_utf8: not support block set!");
        return -1;
    }
}

int plugin_layout_animation_selection_set_text_ucs4(plugin_layout_animation_selection_t selection, const uint32_t * text, size_t text_len) {
    plugin_layout_render_t render = plugin_layout_animation_from_data(selection)->m_render;
    plugin_layout_module_t module = render->m_module;

    plugin_layout_render_do_layout(render);
    
    if (selection->m_type == plugin_layout_animation_selection_line) {
        return plugin_layout_render_update_text_ucs4(render, selection->m_begin_pos, selection->m_end_pos, text, text_len);
    }
    else {
        CPE_ERROR(module->m_em, "plugin_layout_animation_selection_set_text_ucs4: not support block set!");
        return -1;
    }
}

static void plugin_layout_animation_selection_render(
    plugin_layout_animation_t animation, plugin_layout_animation_layer_t layer,
    void * ctx, ui_runtime_render_t context, ui_rect_t clip_rect, ui_runtime_render_second_color_t second_color, ui_transform_t transform)
{
    plugin_layout_render_t render = animation->m_render;
    plugin_layout_animation_selection_t selection;
    plugin_layout_render_node_t node;

    if (layer != plugin_layout_animation_layer_before) return;
    
    selection = plugin_layout_animation_data(animation);
    if (selection->m_begin_pos >= selection->m_end_pos) return;
    
    /*按行绘制 */
    if (selection->m_type == plugin_layout_animation_selection_line) {
        int pos = 0;
        ui_rect render_rect;

        render_rect = UI_RECT_ZERO;
        
        TAILQ_FOREACH(node, &render->m_nodes, m_next) {
            if (pos >= selection->m_begin_pos && pos < selection->m_end_pos) {
                if (ui_rect_width(&render_rect) == 0) {
                    render_rect = node->m_bound_rt;
                }
                else {
                    if (node->m_bound_rt.lt.y > render_rect.rb.y) { /*换行 */
                        /*
                         TODO
                        ui_runtime_render_draw_color_rect(context, clip_rect, transform, &render_rect, &selection->m_color);
                        render_rect = node->m_bound_rt;
                         */
                    }
                    else {
                        ui_rect_inline_union(&render_rect, &node->m_bound_rt);
                    }
                }
            }
            pos++;
        }

        if (ui_rect_is_valid(&render_rect) > 0) {
            //TODO
            //ui_runtime_render_draw_color_rect(context, clip_rect, transform, &render_rect, &selection->m_color);
        }
    }
    else { /*按块绘制 */
        ui_rect select_rect;
        ui_rect render_rect;

        if (selection->m_begin_pt.x == selection->m_end_pt.x && selection->m_begin_pt.y == selection->m_end_pt.y) return;

        select_rect.lt = selection->m_begin_pt;
        select_rect.rb = selection->m_end_pt;

        if (select_rect.lt.x > select_rect.rb.x) { float t = select_rect.lt.x; select_rect.lt.x = select_rect.rb.x; select_rect.rb.x = t; }
        if (select_rect.lt.y > select_rect.rb.y) { float t = select_rect.lt.y; select_rect.lt.y = select_rect.rb.y; select_rect.rb.y = t; }

        render_rect = UI_RECT_ZERO;
        TAILQ_FOREACH(node, &render->m_nodes, m_next) {
            if (ui_rect_is_intersection_valid(&node->m_bound_rt, &select_rect)) {
                if (ui_rect_width(&render_rect) == 0) {
                    render_rect = node->m_bound_rt;
                }
                else {
                    ui_rect_inline_union(&render_rect, &node->m_bound_rt);
                }
            }
        }

        if (ui_rect_is_valid(&render_rect) > 0) {
            /*
             //TODO:
            ui_runtime_render_draw_color_rect(context, clip_rect, transform, &render_rect, &selection->m_color);
             */
        }
    }
}

int plugin_layout_animation_selection_regist(plugin_layout_module_t module) {
    assert(module->m_animation_meta_selection == NULL);

    module->m_animation_meta_selection =
        plugin_layout_animation_meta_create(
            module, "selection", module,
            sizeof(struct plugin_layout_animation_selection),
            plugin_layout_animation_selection_init,
            NULL, /*fini*/
            plugin_layout_animation_selection_layout,
            NULL, /*update*/
            plugin_layout_animation_selection_render);
    
    return module->m_animation_meta_selection ? 0 : -1;
}

void plugin_layout_animation_selection_unregist(plugin_layout_module_t module) {
    assert(module->m_animation_meta_selection);
    plugin_layout_animation_meta_free(module->m_animation_meta_selection);
    module->m_animation_meta_selection = NULL;
}
