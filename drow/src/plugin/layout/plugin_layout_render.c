#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/string_ucs4.h"
#include "render/utils/ui_transform.h"
#include "render/cache/ui_cache_texture.h"
#include "render/runtime/ui_runtime_render_second_color.h"
#include "render/runtime/ui_runtime_render_buff_use.h"
#include "render/runtime/ui_runtime_render_cmd_utils_2d.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "render/runtime/ui_runtime_render_obj_meta.h"
#include "render/runtime/ui_runtime_render.h"
#include "plugin/layout/plugin_layout_utils.h"
#include "plugin_layout_render_i.h"
#include "plugin_layout_render_node_i.h"
#include "plugin_layout_render_group_i.h"
#include "plugin_layout_layout_i.h"
#include "plugin_layout_layout_meta_i.h"
#include "plugin_layout_animation_i.h"
#include "plugin_layout_animation_meta_i.h"
#include "plugin_layout_font_element_i.h"
#include "plugin_layout_font_meta_i.h"

int plugin_layout_render_init(void * ctx, ui_runtime_render_obj_t render_obj) {
    plugin_layout_module_t module = ctx;
    plugin_layout_render_t obj = (plugin_layout_render_t)ui_runtime_render_obj_data(render_obj);

    obj->m_module = module;
    obj->m_layout = NULL;
    obj->m_node_count = 0;
    TAILQ_INIT(&obj->m_nodes);
    TAILQ_INIT(&obj->m_groups);
    TAILQ_INIT(&obj->m_animations);

    obj->m_pos = UI_VECTOR_2_ZERO;
    obj->m_size = UI_VECTOR_2_ZERO;
    obj->m_data = NULL;
    obj->m_data_len = 0;
    obj->m_data_manage = 0;
    obj->m_need_update = 0;
    
    return 0;
}

void plugin_layout_render_free(void * ctx, ui_runtime_render_obj_t render_obj) {
    plugin_layout_module_t module = ctx;
    plugin_layout_render_t obj = (plugin_layout_render_t)ui_runtime_render_obj_data(render_obj);

    if (obj->m_layout) {
        plugin_layout_layout_free(obj->m_layout);
        assert(obj->m_layout == NULL);
    }
    
    if (obj->m_data_manage) {
        assert(obj->m_data);
        mem_free(module->m_alloc, (void*)obj->m_data);
        obj->m_data = NULL;
    }

    while(!TAILQ_EMPTY(&obj->m_nodes)) {
        plugin_layout_render_node_free(TAILQ_FIRST(&obj->m_nodes));
    }
    assert(obj->m_node_count == 0);
    
    while(!TAILQ_EMPTY(&obj->m_groups)) {
        plugin_layout_render_group_free(TAILQ_FIRST(&obj->m_groups));
    }

    while(!TAILQ_EMPTY(&obj->m_animations)) {
        plugin_layout_animation_free(TAILQ_FIRST(&obj->m_animations));
    }
}

plugin_layout_module_t  plugin_layout_render_module(plugin_layout_render_t obj) {
    return obj->m_module;
}

plugin_layout_layout_t plugin_layout_render_layout(plugin_layout_render_t render) {
    return render->m_layout;
}

plugin_layout_layout_t
plugin_layout_render_set_layout(plugin_layout_render_t render, const char * layout_name) {
    plugin_layout_module_t module = render->m_module;
    plugin_layout_layout_meta_t meta;
    plugin_layout_layout_t layout;

    meta = plugin_layout_layout_meta_find(module, layout_name);
    if (meta == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_render_set_layout: layout %s unkonwn", layout_name);
        return NULL;
    }

    layout = plugin_layout_layout_create(render, meta);
    if (layout == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_render_set_layout: layout %s create fail", layout_name);
        return NULL;
    }

    return layout;
}

const char * plugin_layout_render_data(plugin_layout_render_t render) {
    return render->m_data;
}

size_t plugin_layout_render_data_len(plugin_layout_render_t render) {
    return render->m_data_len;
}

int plugin_layout_render_set_data(plugin_layout_render_t render, const char * data) {
    plugin_layout_module_t module = render->m_module;
    char * save_data = NULL;
    size_t data_len = 0;
    
    if (data) {
        if (render->m_data) {
            if (strcmp(data, render->m_data) == 0) return 0;
        }
        
        data_len = strlen(data);
        save_data = cpe_str_mem_dup_len(module->m_alloc, data, data_len);
        if (save_data == NULL) {
            CPE_ERROR(module->m_em, "plugin_layout_render_set_data: dup data %s fail", data);
            return -1;
        }
    }

    if (render->m_data && render->m_data_manage) {
        mem_free(module->m_alloc, (void*)render->m_data);
    }

    render->m_data = save_data;
    render->m_data_len = data_len;
    render->m_data_manage = 1;
    render->m_need_update = 1;

    return render->m_layout->m_meta->m_analize(render->m_layout);    
}

int plugin_layout_render_set_data_ucs4(plugin_layout_render_t render, uint32_t const * text, size_t text_len) {
    plugin_layout_module_t module = render->m_module;
    char * save_data = NULL;

    if (text) {
        save_data = cpe_str_utf8_from_ucs4_len(module->m_alloc, text, text_len);
        if (save_data == NULL) {
            CPE_ERROR(module->m_em, "plugin_layout_render_set_data: dup data fail");
            return -1;
        }
    }
    else {
        assert(text_len == 0);
    }

    if (save_data == render->m_data) {
        if (strcmp(save_data, render->m_data) == 0) {
            mem_free(module->m_alloc, save_data);
            return 0;
        }
    }
    
    if (render->m_data && render->m_data_manage) {
        mem_free(module->m_alloc, (void*)render->m_data);
    }

    render->m_data = save_data;
    render->m_data_len = strlen(render->m_data);
    render->m_data_manage = 1;
    render->m_need_update = 1;

    return render->m_layout->m_meta->m_analize(render->m_layout);    
}

int plugin_layout_render_set_data_extern(plugin_layout_render_t render, const char * data) {
    plugin_layout_module_t module = render->m_module;
    
    if (render->m_data && render->m_data_manage) {
        mem_free(module->m_alloc, (void*)render->m_data);
    }

    render->m_data = data;
    render->m_data_manage = 0;
    render->m_need_update = 1;

    return render->m_layout->m_meta->m_analize(render->m_layout);
}


ui_vector_2_t plugin_layout_render_pos(plugin_layout_render_t render) {
    return &render->m_pos;
}

void plugin_layout_render_set_pos(plugin_layout_render_t render, ui_vector_2_t pos) {
    if (ui_vector_2_cmp(&render->m_pos, pos) != 0) {
        render->m_pos = *pos;
    }
}

ui_vector_2_t plugin_layout_render_size(plugin_layout_render_t render) {
    return &render->m_size;
}

void plugin_layout_render_set_size(plugin_layout_render_t render, ui_vector_2_t size) {
	if (ui_vector_2_cmp(&render->m_size, size) != 0) {
		render->m_size = *size;
		render->m_need_update = 1;
	}
}

void plugin_layout_render_clear_nodes(plugin_layout_render_t render) {
    while(!TAILQ_EMPTY(&render->m_nodes)) {
        plugin_layout_render_node_free(TAILQ_FIRST(&render->m_nodes));
    }
}

void plugin_layout_render_bound_rt(plugin_layout_render_t render, ui_rect_t rt) {
    plugin_layout_render_node_t node;

    for(node = TAILQ_FIRST(&render->m_nodes); node; node = TAILQ_NEXT(node, m_next)) {
        if (node == TAILQ_FIRST(&render->m_nodes)) {
            *rt = node->m_bound_rt;
        }
        else {
            ui_rect_inline_union(rt, &node->m_bound_rt);
        }
    }
}

void plugin_layout_render_adj_pos(plugin_layout_render_t render, ui_vector_2_t adj_pos) {
    plugin_layout_render_node_t node;

    TAILQ_FOREACH(node, &render->m_nodes, m_next) {
        plugin_layout_render_node_adj_pos(node, adj_pos);
    }
}

void plugin_layout_render_do_layout(plugin_layout_render_t render) {
    if (render->m_need_update && render->m_layout) {
        plugin_layout_animation_t animation;
        
        assert(render->m_layout->m_meta->m_layout);
        plugin_layout_render_clear_nodes(render);
        if (render->m_layout->m_meta->m_layout(render->m_layout) == 0) {
            render->m_need_update = 0;
        }

        TAILQ_FOREACH(animation, &render->m_animations, m_next_for_render) {
            if (animation->m_meta->m_layout_fun) {
                animation->m_meta->m_layout_fun(animation, animation->m_meta->m_ctx);
            }
        }
    }
}

uint32_t plugin_layout_render_node_count(plugin_layout_render_t render) {
    return render->m_node_count;
}

cpe_str_ucs4_t plugin_layout_render_text_ucs4(mem_allocrator_t alloc, plugin_layout_render_t render) {
    struct plugin_layout_render_node_it node_it;

    plugin_layout_render_do_layout(render);
    plugin_layout_render_nodes(render, &node_it);

    return plugin_layout_render_nodes_to_ucs4(alloc, &node_it);
}

char * plugin_layout_render_text_utf8(mem_allocrator_t alloc, plugin_layout_render_t render) {
    struct plugin_layout_render_node_it node_it;

    plugin_layout_render_do_layout(render);
    plugin_layout_render_nodes(render, &node_it);

    return plugin_layout_render_nodes_to_utf8(alloc, &node_it);
}

static plugin_layout_render_node_t plugin_layout_render_node_next(struct plugin_layout_render_node_it * it) {
    plugin_layout_render_node_t * data = (plugin_layout_render_node_t *)(it->m_data);
    plugin_layout_render_node_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next);

    return r;
}

void plugin_layout_render_nodes(plugin_layout_render_t render, plugin_layout_render_node_it_t it) {
    *(plugin_layout_render_node_t *)(it->m_data) = TAILQ_FIRST(&render->m_nodes);
    it->next = plugin_layout_render_node_next;
}

int plugin_layout_render_update_text_ucs4(plugin_layout_render_t render, int begin_pos, int end_pos, uint32_t const * text, size_t text_len) {
    plugin_layout_layout_t layout = render->m_layout;
    
    if (layout->m_meta->m_update == NULL) {
        CPE_ERROR(render->m_module->m_em, "plugin_layout_render_update_text_utf8: layout %s not support update!", layout->m_meta->m_name);
        return -1;
    }
    
    return layout->m_meta->m_update(layout, begin_pos, end_pos, text, text_len);
}

int plugin_layout_render_update_text_utf8(plugin_layout_render_t render, int begin_pos, int end_pos, const char * text) {
    cpe_str_ucs4_t ucs_str;
    plugin_layout_layout_t layout = render->m_layout;

    if (layout->m_meta->m_update == NULL) {
        CPE_ERROR(render->m_module->m_em, "plugin_layout_render_update_text_utf8: layout %s not support update!", layout->m_meta->m_name);
        return -1;
    }
    
    ucs_str = cpe_str_ucs4_from_utf8(render->m_module->m_alloc, text);
    if (ucs_str == NULL) {
        CPE_ERROR(render->m_module->m_em, "plugin_layout_render_update_text_utf8: utf8 to ucs4 fail!");
        return -1;
    }

    return layout->m_meta->m_update(layout, begin_pos, end_pos, cpe_str_ucs4_data(ucs_str), cpe_str_ucs4_len(ucs_str));
}

static void plugin_layout_render_update(void * ctx, ui_runtime_render_obj_t render_obj, float delta) {
    plugin_layout_render_t render = ui_runtime_render_obj_data(render_obj);
    plugin_layout_animation_t anim;
    
    plugin_layout_render_do_layout(render);

    TAILQ_FOREACH(anim, &render->m_animations, m_next_for_render) {
        if (anim->m_meta->m_update_fun) {
            anim->m_meta->m_update_fun(anim, anim->m_meta->m_ctx, delta);
        }
    }
}

static int plugin_layout_render_render(
    void * ctx, ui_runtime_render_obj_t render_obj,
    ui_runtime_render_t render, ui_rect_t clip_rect, ui_runtime_render_second_color_t second_color, ui_transform_t transform)
{
    plugin_layout_render_t layout_render = ui_runtime_render_obj_data(render_obj);
    plugin_layout_animation_t anim;
    plugin_layout_render_node_t node;
    ui_runtime_render_cmd_t cmd;
    
    plugin_layout_render_do_layout(layout_render);
    
    assert(second_color);

    TAILQ_FOREACH(anim, &layout_render->m_animations, m_next_for_render) {
        if (anim->m_meta->m_render_fun) {
            anim->m_meta->m_render_fun(
                anim, plugin_layout_animation_layer_before,
                anim->m_meta->m_ctx, render, clip_rect, second_color, transform);
        }
    }

    cmd = NULL;
    TAILQ_FOREACH(node, &layout_render->m_nodes, m_next) {
        if (node->m_element) {
            plugin_layout_font_meta_t font_meta = node->m_element->m_face->m_meta;
            
            font_meta->m_render_element(
                font_meta->m_ctx,
                node->m_element, &node->m_font_draw, &node->m_render_rt,
                render, &cmd, clip_rect, second_color, transform);
        }
    }
    ui_runtime_render_cmd_quad_batch_commit(&cmd, render);
    
    TAILQ_FOREACH(anim, &layout_render->m_animations, m_next_for_render) {
        if (anim->m_meta->m_render_fun) {
            anim->m_meta->m_render_fun(
                anim, plugin_layout_animation_layer_after,
                anim->m_meta->m_ctx, render, clip_rect, second_color, transform);
        }
    }
    
    return 0;
}

static int plugin_layout_render_setup(void * ctx, ui_runtime_render_obj_t render_obj, char * args) {
    plugin_layout_render_t render = ui_runtime_render_obj_data(render_obj);
    const char * str_value;
    plugin_layout_layout_t layout;
    
    str_value = cpe_str_read_and_remove_arg(args, "layout", ',', '=');
    if (str_value == NULL) str_value = "basic";

    layout = plugin_layout_render_set_layout(render, str_value);
    if (layout == NULL) return -1;

    if (plugin_layout_layout_setup(layout, args) != 0) return -1;
    
    return 0;
}

static int plugin_layout_render_resize(void * ctx, ui_runtime_render_obj_t render_obj, ui_vector_2_t size) {
    plugin_layout_render_t render = ui_runtime_render_obj_data(render_obj);

    if (ui_vector_2_cmp(&render->m_size, size) != 0) {
        render->m_size = *size;
        render->m_need_update = 1;
    }

    return 0;
}

int plugin_layout_render_register(plugin_layout_module_t module) {
    ui_runtime_render_obj_meta_t obj_meta;
        
    if (module->m_runtime == NULL) return 0;
    
    obj_meta =
        ui_runtime_render_obj_meta_create(
            module->m_runtime, "layout", 0, sizeof(struct plugin_layout_render), module,
            plugin_layout_render_init,
            NULL,
            plugin_layout_render_setup,
            plugin_layout_render_update,
            plugin_layout_render_free,
            plugin_layout_render_render,
            NULL,
            NULL,
            plugin_layout_render_resize);
    if (obj_meta == NULL) {
        CPE_ERROR(module->m_em, "%s: create: register render obj layout fail", plugin_layout_module_name(module));
        return -1;
    }

    return 0;
}

void plugin_layout_render_unregister(plugin_layout_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_obj_meta_t obj_meta = ui_runtime_render_obj_meta_find_by_name(module->m_runtime, "layout");
        if (obj_meta) {
            ui_runtime_render_obj_meta_free(obj_meta);
        }
    }
}

void plugin_layout_render_render_element(
    ui_rect_t target_rect, ui_rect_t texture_rect, ui_cache_res_t texture, ui_runtime_render_texture_filter_t filter,
    ui_runtime_render_program_buildin_t program, ui_color_t blend_color, 
    ui_runtime_render_t render, ui_runtime_render_cmd_t * batch_cmd,
    ui_rect_t clip_rect, ui_runtime_render_second_color_t second_color, ui_transform_t transform)
{
    int i;
    uint32_t abgr;
    struct ui_color input_color = second_color->m_color;
    ui_vector_2 points[] = {
        UI_VECTOR_2_INITLIZER(target_rect->lt.x, target_rect->lt.y),
        UI_VECTOR_2_INITLIZER(target_rect->rb.x, target_rect->rb.y),
        UI_VECTOR_2_INITLIZER(target_rect->lt.x, target_rect->rb.y),
        UI_VECTOR_2_INITLIZER(target_rect->rb.x, target_rect->lt.y)
    };
    ui_vector_2 texture_sz = UI_VECTOR_2_INITLIZER( (float)ui_cache_texture_width(texture), (float)ui_cache_texture_height(texture));
    //ui_vector_2 uvs[] = {
    //    UI_VECTOR_2_INITLIZER((texture_rect->lt.x + 0.375f) / texture_sz.x, (texture_rect->lt.y + 0.375f) / texture_sz.y),
    //    UI_VECTOR_2_INITLIZER((texture_rect->rb.x - 0.375f) / texture_sz.x, (texture_rect->rb.y - 0.375f) / texture_sz.y),
    //    UI_VECTOR_2_INITLIZER((texture_rect->lt.x + 0.375f) / texture_sz.x, (texture_rect->rb.y - 0.375f) / texture_sz.y),
    //    UI_VECTOR_2_INITLIZER((texture_rect->rb.x - 0.375f) / texture_sz.x, (texture_rect->lt.y + 0.375f) / texture_sz.y)
    //};
    //fixed bugs
    ui_vector_2 uvs[] = {
        UI_VECTOR_2_INITLIZER((texture_rect->lt.x) / texture_sz.x, (texture_rect->lt.y) / texture_sz.y),
        UI_VECTOR_2_INITLIZER((texture_rect->rb.x) / texture_sz.x, (texture_rect->rb.y) / texture_sz.y),
        UI_VECTOR_2_INITLIZER((texture_rect->lt.x) / texture_sz.x, (texture_rect->rb.y) / texture_sz.y),
        UI_VECTOR_2_INITLIZER((texture_rect->rb.x) / texture_sz.x, (texture_rect->lt.y) / texture_sz.y)
    };

    for(i = 0; i < CPE_ARRAY_SIZE(points); ++i) {
        ui_transform_inline_adj_vector_2(transform, &points[i]);
    }

    if (clip_rect) {
        ui_vector_2 origin_sz = UI_VECTOR_2_INITLIZER(ui_rect_width(target_rect), ui_rect_height(target_rect));
        ui_vector_2 uv_adj = UI_VECTOR_2_INITLIZER(
            ((1.0f / transform->m_s.x) / origin_sz.x) * (ui_rect_width(texture_rect) / texture_sz.x),
            ((1.0f / transform->m_s.y) / origin_sz.y) * (ui_rect_height(texture_rect) / texture_sz.y));

        for(i = 0; i < CPE_ARRAY_SIZE(points); ++i) {
            ui_vector_2_t pt = &points[i];
            ui_vector_2 origin;
            
            origin = *pt;
            
            if		(pt->x < clip_rect->lt.x) pt->x = clip_rect->lt.x;
            else if (pt->x > clip_rect->rb.x) pt->x = clip_rect->rb.x;
            if		(pt->y < clip_rect->lt.y) pt->y = clip_rect->lt.y;
            else if (pt->y > clip_rect->rb.y) pt->y = clip_rect->rb.y;

            uvs[i].x += (pt->x - origin.x) * uv_adj.x;
            uvs[i].y += (pt->y - origin.y) * uv_adj.y;
        }

        /* 完全剪裁 */
        if (points[2].y == points[0].y || points[3].x == points[0].x) {
            return;
        }
    }

    /*计算混合 */
    if (second_color->m_mix == ui_runtime_render_second_color_add) {
        if (program == ui_runtime_render_program_buildin_add) {
            ui_color t_color = input_color;
            ui_color_inline_add(&t_color, blend_color);
            abgr = ui_color_make_abgr(&t_color);
        }
        else if (program == ui_runtime_render_program_buildin_multiply) {
            ui_color t_color = input_color;
            ui_color_inline_mul(&t_color, blend_color);
            abgr = ui_color_make_abgr(&t_color);
        }
        else {
            abgr = ui_color_make_abgr(&input_color);
        }
    }
    else if (second_color->m_mix == ui_runtime_render_second_color_multiply) {
        ui_color t_color = input_color;
        ui_color_inline_mul(&t_color, blend_color);
        abgr = ui_color_make_abgr(&t_color);
    }
    else {
        if (program == ui_runtime_render_program_buildin_add) {
            abgr = ui_color_make_abgr(blend_color);
        }
        else if (program == ui_runtime_render_program_buildin_multiply) {
            abgr = ui_color_make_abgr(blend_color);
        }
        else {
            //abgr = ui_color_make_abgr(blend_color);
            abgr = 0xFFFFFFFFu;
        }
    }
        
    /*开始绘制 */
    do {
        struct ui_runtime_render_blend blend = { ui_runtime_render_src_alpha, ui_runtime_render_one_minus_src_alpha };
        ui_runtime_vertex_v3f_t2f_c4ub vertexs[4] = {
            { UI_VECTOR_3_INITLIZER(points[0].x, points[0].y, 0.0f), UI_VECTOR_2_INITLIZER(uvs[0].x, uvs[0].y), abgr },
            { UI_VECTOR_3_INITLIZER(points[1].x, points[1].y, 0.0f), UI_VECTOR_2_INITLIZER(uvs[1].x, uvs[1].y), abgr },
            { UI_VECTOR_3_INITLIZER(points[2].x, points[2].y, 0.0f), UI_VECTOR_2_INITLIZER(uvs[2].x, uvs[2].y), abgr },
            { UI_VECTOR_3_INITLIZER(points[3].x, points[3].y, 0.0f), UI_VECTOR_2_INITLIZER(uvs[3].x, uvs[3].y), abgr },
        };

        ui_runtime_render_cmd_quad_batch_append(
            batch_cmd,
            render, 0.0f, NULL,
            vertexs,
            texture,
            filter,
            program, 
            &blend);
    } while(0);
}

