#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_color.h"
#include "render/cache/ui_cache_color.h"
#include "render/runtime/ui_runtime_render_utils.h"
#include "plugin/layout/plugin_layout_animation_meta.h"
#include "plugin/layout/plugin_layout_animation.h"
#include "plugin_layout_animation_caret_i.h"
#include "plugin_layout_animation_i.h"
#include "plugin_layout_render_i.h"
#include "plugin_layout_render_node_i.h"
#include "plugin_layout_layout_basic_i.h"
#include "plugin_layout_layout_rich_i.h"
#include "plugin_layout_font_face_i.h"

plugin_layout_animation_caret_t
plugin_layout_animation_caret_create(plugin_layout_render_t render) {
    plugin_layout_animation_t animation;

    animation = plugin_layout_animation_create(render, render->m_module->m_animation_meta_caret);
    if (animation == NULL) return NULL;

    return plugin_layout_animation_data(animation);
}

void plugin_layout_animation_caret_free(plugin_layout_animation_caret_t caret) {
    plugin_layout_animation_t animation = plugin_layout_animation_from_data(caret);
    plugin_layout_animation_free(animation);
}

plugin_layout_animation_caret_t plugin_layout_animation_caret_from_animation(plugin_layout_animation_t animation) {
    return plugin_layout_animation_data(animation);
}

plugin_layout_animation_caret_t plugin_layout_animation_caret_find_first(plugin_layout_render_t render) {
    plugin_layout_animation_t animation;

    animation = plugin_layout_animation_find_first_by_type(render, "caret");

    return animation ? plugin_layout_animation_caret_from_animation(animation) : NULL;
}

int plugin_layout_animation_caret_pos(plugin_layout_animation_caret_t caret) {
    return caret->m_pos;
}

void plugin_layout_animation_caret_set_pos(plugin_layout_animation_caret_t caret, int pos) {
    caret->m_pos = pos;
}

uint8_t plugin_layout_animation_caret_is_visiable(plugin_layout_animation_caret_t caret) {
    return caret->m_is_visiable;
}

void plugin_layout_animation_caret_set_visiable(plugin_layout_animation_caret_t caret, uint8_t is_visiable) {
    caret->m_is_visiable = is_visiable ? 1 : 0;
}

void plugin_layout_animation_caret_set_pos_by_pt(plugin_layout_animation_caret_t caret, ui_vector_2_t pt) {
    plugin_layout_animation_t animation = plugin_layout_animation_from_data(caret);
    plugin_layout_render_t render = animation->m_render;
    plugin_layout_render_node_t node;
    int index;

    plugin_layout_render_do_layout(render);    

    index = 0;
    TAILQ_FOREACH(node, &animation->m_render->m_nodes, m_next) {
        if (ui_rect_is_contain_pt(&node->m_bound_rt, pt)) {
            break;
        }
        index++;
    }

    if (node == NULL) {
        if (pt->x < 0) {
            index = 0;
        }
        else {
            index = -1;
        }
    }
    else {
        if (pt->x > (node->m_bound_rt.lt.x + node->m_bound_rt.rb.x) / 2.0f) {
            index++;
        }
    }

    caret->m_pos = index;
}

static int plugin_layout_animation_caret_init(plugin_layout_animation_t animation, void * ctx) {
    plugin_layout_module_t module = animation->m_render->m_module;
    plugin_layout_animation_caret_t caret;
    
    caret = plugin_layout_animation_data(animation);
    
    if (ui_cache_find_color(module->m_cache_mgr, "caret", &caret->m_color) != 0) {
        if (ui_cache_find_color(module->m_cache_mgr, "black", &caret->m_color) != 0) {
            CPE_ERROR(module->m_em, "plugin_layout_animation_caret_init: no caret color 'caret' or 'black'");
            return -1;
        }
    }

    caret->m_pos = -1;
    caret->m_is_show = 1;
    caret->m_is_visiable = 1;
    caret->m_update_time = 0.0f;
    
    return 0;
}

static uint8_t plugin_layout_animation_caret_update(plugin_layout_animation_t animation, void * ctx, float delta_s) {
    plugin_layout_animation_caret_t caret;
    
    caret = plugin_layout_animation_data(animation);

	caret->m_update_time += delta_s;
	if (caret->m_update_time > 0.5f) {
		caret->m_is_show = caret->m_is_show ? 0 : 1;
		caret->m_update_time = 0.0f;
	}
    
    return 1;
}

static void plugin_layout_animation_caret_render(
    plugin_layout_animation_t animation, plugin_layout_animation_layer_t layer,
    void * ctx, ui_runtime_render_t context, ui_rect_t clip_rect, ui_runtime_render_second_color_t second_color, ui_transform_t transform)
{
    plugin_layout_module_t module = ctx;
    plugin_layout_render_t render = animation->m_render;
    plugin_layout_animation_caret_t caret;
    ui_rect rect;

    if (layer != plugin_layout_animation_layer_after) return;
    
    caret = plugin_layout_animation_data(animation);

    if (!caret->m_is_visiable) return;
    if (!caret->m_is_show) return;

    rect = UI_RECT_ZERO;
    
    if (caret->m_pos < 0) {
        plugin_layout_render_node_t node = TAILQ_LAST(&render->m_nodes, plugin_layout_render_node_list);
        if (node) {
            rect.lt.x = node->m_bound_rt.rb.x;
            rect.lt.y = node->m_bound_rt.lt.y;
            rect.rb.x = node->m_bound_rt.rb.x + 1;
            rect.rb.y = node->m_bound_rt.rb.y;
        }
    }
    else {
        plugin_layout_render_node_t node;
        int i = 0;
        for(node = TAILQ_FIRST(&animation->m_render->m_nodes);
            node && i < caret->m_pos;
            i++, node = TAILQ_NEXT(node, m_next))
        {
        }

        if (node) {
            rect.lt.x = node->m_bound_rt.lt.x;
            rect.lt.y = node->m_bound_rt.lt.y;
            rect.rb.x = node->m_bound_rt.lt.x + 1;
            rect.rb.y = node->m_bound_rt.rb.y;
        }
        else {
            plugin_layout_render_node_t node = TAILQ_LAST(&render->m_nodes, plugin_layout_render_node_list);
            if (node) {
                rect.lt.x = node->m_bound_rt.rb.x;
                rect.lt.y = node->m_bound_rt.lt.y;
                rect.rb.x = node->m_bound_rt.rb.x + 1;
                rect.rb.y = node->m_bound_rt.rb.y;
            }
        }
    }

    if (ui_rect_width(&rect) == 0.0f) {
        plugin_layout_font_face_t face = NULL;
        
        rect.lt.x = 0;
        rect.lt.y = 0;
        rect.rb.x = rect.lt.x + 1;

        if (strcmp(plugin_layout_layout_meta_name(render->m_layout), "basic") == 0) {
            plugin_layout_layout_basic_t basic = plugin_layout_layout_data(render->m_layout);
            face = plugin_layout_font_face_find(module, &basic->m_font_id);
        }
        else if (strcmp(plugin_layout_layout_meta_name(render->m_layout), "rich") == 0) {
            plugin_layout_layout_rich_t rich = plugin_layout_layout_data(render->m_layout);
            face = plugin_layout_font_face_find(module, &rich->m_default_font_id);
        }

        if (face) {
            rect.rb.y = rect.lt.y + face->m_height;
        }
        else {
            rect.rb.y = rect.lt.y + 12;
        }
    }

    //TODO:
    //ui_runtime_render_draw_color_rect(context, clip_rect, transform, &rect, &caret->m_color);
}

int plugin_layout_animation_caret_regist(plugin_layout_module_t module) {
    assert(module->m_animation_meta_caret == NULL);

    module->m_animation_meta_caret =
        plugin_layout_animation_meta_create(
            module, "caret", module,
            sizeof(struct plugin_layout_animation_caret),
            plugin_layout_animation_caret_init,
            NULL,
            NULL,
            plugin_layout_animation_caret_update,
            plugin_layout_animation_caret_render);
    
    return module->m_animation_meta_caret ? 0 : -1;
}

void plugin_layout_animation_caret_unregist(plugin_layout_module_t module) {
    assert(module->m_animation_meta_caret);
    plugin_layout_animation_meta_free(module->m_animation_meta_caret);
    module->m_animation_meta_caret = NULL;
}
