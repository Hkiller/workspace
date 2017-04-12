#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "protocol/render/model/ui_object_ref.h"
#include "render/utils/ui_transform.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_layout.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_meta.h"
#include "plugin_ui_template_render_i.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_page_i.h"

static int plugin_ui_template_render_do_init(void * ctx, ui_runtime_render_obj_t render_obj) {
    plugin_ui_template_render_t obj = ui_runtime_render_obj_data(render_obj);
    obj->m_control = NULL;
    obj->m_is_free = 0;
    obj->m_pos_policy = ui_pos_policy_center;
    return 0;
}

static int plugin_ui_template_render_do_set(void * ctx, ui_runtime_render_obj_t render_obj, UI_OBJECT_URL const * obj_url) {
    plugin_ui_module_t module = ctx;
    plugin_ui_template_render_t obj = ui_runtime_render_obj_data(render_obj);
    UI_OBJECT_URL_DATA_UI_TEMPLATE const * template_data = &obj_url->data.ui_template;
    plugin_ui_control_t new_control = NULL;
    plugin_ui_env_t env = TAILQ_FIRST(&module->m_envs);
    ui_data_src_t layout_src;
    ui_data_layout_t layout;
    ui_data_control_t data_control;

    if (env == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_template_do_set: no env!");
        return -1;
    }

    layout_src = ui_data_src_find_by_path(module->m_data_mgr, template_data->src.data.by_path.path, ui_data_src_type_layout);
    if (layout_src == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_template_do_set: layout %s not exist!", template_data->src.data.by_path.path);
        return -1;
    }

    layout = ui_data_src_product(layout_src);
    if (layout == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_template_do_set: layout %s not loaded!", template_data->src.data.by_path.path);
        return -1;
    }

    if (env->m_template_page == NULL) {
        env->m_template_page = plugin_ui_page_create(env, NULL, NULL);
        if (env->m_template_page == NULL) {
            CPE_ERROR(module->m_em, "plugin_ui_template_do_set: create template page fail!");
            return -1;
        }
    }

    data_control = ui_data_layout_root(layout);
    assert(data_control);

    new_control = plugin_ui_control_create(env->m_template_page, ui_data_control_type(data_control));
    if (new_control == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_template_do_set: create template root control fail!");
        return -1;
    }

    if (plugin_ui_control_set_template(new_control, data_control) != 0) {
        CPE_ERROR(module->m_em, "plugin_ui_template_do_set: load template data fail!");
        plugin_ui_control_free(new_control);
        return -1;
    }

    plugin_ui_control_add_child_tail(plugin_ui_page_root_control(env->m_template_page), new_control);
    
    if (obj->m_control) {
        plugin_ui_control_free(obj->m_control);
    }
    
    obj->m_control = new_control;
    
    return 0;
}

static void plugin_ui_template_render_do_free(void * ctx, ui_runtime_render_obj_t render_obj) {
    plugin_ui_template_render_t obj = ui_runtime_render_obj_data(render_obj);

    if (obj->m_control) {
        plugin_ui_control_free(obj->m_control);
        obj->m_control = NULL;
    }
}

static void plugin_ui_template_render_do_update(void * ctx, ui_runtime_render_obj_t render_obj, float delta) {
    plugin_ui_template_render_t obj = ui_runtime_render_obj_data(render_obj);

    if (obj->m_control) {
        plugin_ui_control_visit_tree(
            obj->m_control, plugin_ui_control_visit_tree_bfs, &delta, plugin_ui_control_do_update, 1);
    }
}

static int plugin_ui_template_render_do_render(
    void * ctx, ui_runtime_render_obj_t render_obj,
    ui_runtime_render_t context, ui_rect_t clip_rect,
    ui_runtime_render_second_color_t second_color, ui_transform_t transform)
{
    plugin_ui_template_render_t obj = ui_runtime_render_obj_data(render_obj);
    
    if (obj->m_control == NULL) return 0;

    if (!obj->m_is_free) {
        ui_vector_2 sz;
        ui_rect screen_rect;
    
        /*计算控件大小 */
        sz = plugin_ui_control_calc_child_real_sz_no_scale(obj->m_control->m_parent, plugin_ui_control_template(obj->m_control));

        sz.x *= transform->m_s.x;
        sz.y *= transform->m_s.y;

        /*确定基础位置 */
        ui_transform_get_pos_2(transform, &screen_rect.lt);
    
        /*根据位置策略调整基础位置 */
        switch(obj->m_pos_policy) {
        case ui_pos_policy_top_center:
            screen_rect.lt.x -= sz.x / 2;
            break;
        case ui_pos_policy_top_right:
            screen_rect.lt.x -= sz.x;
            break;
        case ui_pos_policy_center_left:
            screen_rect.lt.y -= sz.y / 2;
            break;
        case ui_pos_policy_center:
            screen_rect.lt.y -= sz.y / 2;
            screen_rect.lt.x -= sz.x / 2;
            break;
        case ui_pos_policy_center_right:
            screen_rect.lt.y -= sz.y / 2;
            screen_rect.lt.x -= sz.x;
            break;
        case ui_pos_policy_bottom_left:
            screen_rect.lt.y -= sz.y;
            break;
        case ui_pos_policy_bottom_center:
            screen_rect.lt.y -= sz.y;
            screen_rect.lt.x -= sz.x / 2;
            break;
        case ui_pos_policy_bottom_right:
            screen_rect.lt.y -= sz.y;
            screen_rect.lt.x -= sz.x;
            break;
        case ui_pos_policy_top_left:
        default:
            break;
        }

        /*放置控件 */
        screen_rect.rb.x = screen_rect.lt.x + sz.x;
        screen_rect.rb.y = screen_rect.lt.y + sz.y;

        plugin_ui_control_place_on_screen(obj->m_control, &screen_rect.lt, &screen_rect);
    }
    
    plugin_ui_control_render_tree(obj->m_control, context, clip_rect);
    
    return 0;
}

plugin_ui_control_t plugin_ui_template_render_control(plugin_ui_template_render_t render) {
    return render->m_control;
}

uint8_t plugin_ui_template_render_is_free(plugin_ui_template_render_t render) {
    return render->m_is_free;
}

void plugin_ui_template_render_set_is_free(plugin_ui_template_render_t render, uint8_t is_free) {
    render->m_is_free = is_free;
}

int plugin_ui_template_render_regist(plugin_ui_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_obj_meta_t obj_meta;

        obj_meta =
            ui_runtime_render_obj_meta_create(
                module->m_runtime, "ui-template", UI_OBJECT_TYPE_UI_TEMPLATE, sizeof(struct plugin_ui_template_render), module,
                plugin_ui_template_render_do_init,
                plugin_ui_template_render_do_set,
                NULL,
                plugin_ui_template_render_do_update,
                plugin_ui_template_render_do_free,
                plugin_ui_template_render_do_render,
                NULL,
                NULL,
                NULL);
        if (obj_meta == NULL) {
            CPE_ERROR(module->m_em, "plugin_ui_template_render_regist: register render obj fail");
            return -1;
        }
    }

    return 0;
}

void plugin_ui_template_render_unregist(plugin_ui_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_obj_meta_t obj_meta = NULL;

        if ((obj_meta = ui_runtime_render_obj_meta_find_by_id(module->m_runtime, UI_OBJECT_TYPE_UI_TEMPLATE))) {
            ui_runtime_render_obj_meta_free(obj_meta);
        }
    }
}
