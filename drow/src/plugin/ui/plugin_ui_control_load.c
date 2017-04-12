#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_layout.h"
#include "render/runtime/ui_runtime_module.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_control_frame_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_utils_i.h"

ui_data_control_t plugin_ui_control_find_template(plugin_ui_module_t module, const char * res) {
    ui_data_src_t layout_src;
    ui_data_layout_t layout;
    ui_data_control_t r;

    layout_src = ui_data_src_find_by_path(module->m_data_mgr, res, ui_data_src_type_layout);
    if (layout_src == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_control_find_template: template %s not exist!", res);
        return NULL;
    }

    layout = ui_data_src_product(layout_src);
    if (layout == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_control_find_template: template %s not loaded!", res); 
        return NULL;
    }

    r = ui_data_layout_root(layout);
    if (r == NULL) {
        CPE_ERROR(module->m_em, "plugin_ui_control_find_template: template %s no root control!", res); 
        return NULL;
    }
    
    return r;
}

ui_data_control_t plugin_ui_control_template(plugin_ui_control_t control) {
    return control->m_template;
}

plugin_ui_control_t plugin_ui_control_template_root(plugin_ui_control_t control) {
    plugin_ui_control_t c = control;

    while(c->m_template == NULL) c = c->m_parent;

    return c;
}

static int plugin_ui_control_do_load_tree(plugin_ui_control_t control) {
    plugin_ui_module_t module = control->m_page->m_env->m_module;
    UI_CONTROL const * self_data;
    ui_data_control_t template;
    UI_CONTROL const * template_data;
    struct ui_data_control_addition_it addition_it;
    ui_data_control_addition_t addition;
    
    self_data =
        control->m_src ? ui_data_control_data(control->m_src)
        : control->m_template ? ui_data_control_data(control->m_template)
        : NULL;
    assert(self_data);

    template = control->m_template ? control->m_template : control->m_src;
    assert(template);
    
    template_data = ui_data_control_data(template);

    /*加载本地控件优先的数据 */
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_visible, self_data->basic.visible ? cpe_ba_true : cpe_ba_false);
    
    control->m_render_pt = self_data->basic.render_pt;
    ui_assert_unit_vector_2_sane(&control->m_render_pt);

    control->m_editor_pt.x = self_data->basic.editor_pt.value[0];
    control->m_editor_pt.y = self_data->basic.editor_pt.value[1];

    control->m_align_horz = self_data->basic.align_horz;
    control->m_align_vert = self_data->basic.align_vert;

    /*加载模板优先的数据 */
    control->m_editor_sz.x = template_data->basic.editor_sz.value[0];
    control->m_editor_sz.y = template_data->basic.editor_sz.value[1];
    control->m_editor_pd.lt.x = template_data->basic.editor_pd.lt;
    control->m_editor_pd.lt.y = template_data->basic.editor_pd.tp;
    control->m_editor_pd.rb.x = template_data->basic.editor_pd.rt;
    control->m_editor_pd.rb.y = template_data->basic.editor_pd.bm;

    control->m_render_sz = template_data->basic.render_sz;
    control->m_client_sz = template_data->basic.client_sz;
    control->m_client_pd = template_data->basic.client_pd;

    control->m_pivot.x = template_data->basic.pivot.value[0];
    control->m_pivot.y = template_data->basic.pivot.value[1];
    
    control->m_scale.x = template_data->basic.scale.value[0];
    control->m_scale.y = template_data->basic.scale.value[1];

    control->m_float_scale.x = template_data->basic.float_scale.value[0];
    control->m_float_scale.y = template_data->basic.float_scale.value[1];
    
    control->m_angle.x = template_data->basic.angle.value[0];
    control->m_angle.y = template_data->basic.angle.value[1];
    control->m_angle.z = template_data->basic.angle.value[2];

    control->m_alpha = template_data->basic.alpha;

    control->m_color.a = template_data->basic.color.a;
    control->m_color.r = template_data->basic.color.r;
    control->m_color.g = template_data->basic.color.g;
    control->m_color.b = template_data->basic.color.b;

    control->m_gray_color.a = template_data->basic.gray_color.a;
    control->m_gray_color.r = template_data->basic.gray_color.r;
    control->m_gray_color.g = template_data->basic.gray_color.g;
    control->m_gray_color.b = template_data->basic.gray_color.b;

    control->m_draw_align = template_data->basic.draw_align;
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_draw_inner, template_data->basic.draw_inner ? cpe_ba_true : cpe_ba_false);
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_enable, template_data->basic.enable ? cpe_ba_true : cpe_ba_false);
    
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_accept_ptls, template_data->basic.accept_pt_ls ? cpe_ba_true : cpe_ba_false);
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_accept_szls, template_data->basic.accept_sz_ls ? cpe_ba_true : cpe_ba_false);
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_accept_click, template_data->basic.accept_hits ? cpe_ba_true : cpe_ba_false);
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_accept_clip, template_data->basic.accept_clip ? cpe_ba_true : cpe_ba_false);
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_parent_clip, template_data->basic.parent_clip ? cpe_ba_true : cpe_ba_false);
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_accept_move, template_data->basic.accept_move ? cpe_ba_true : cpe_ba_false);
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_force_clicp, template_data->basic.force_clip ? cpe_ba_true : cpe_ba_false);
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_accept_double_click, template_data->basic.accept_double_click ? cpe_ba_true : cpe_ba_false);
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_accept_click_move, template_data->basic.accept_click_move ? cpe_ba_true : cpe_ba_false);
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_accept_float, template_data->basic.accept_float ? cpe_ba_true : cpe_ba_false);
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_accept_global_disable_color, template_data->basic.accept_global_down_color ? cpe_ba_true : cpe_ba_false);

    plugin_ui_control_set_draw_frame(control, template_data->basic.draw_frame);
    plugin_ui_control_set_draw_color(control, template_data->basic.draw_color);

    ui_data_control_additions(&addition_it, template);
    while((addition = ui_data_control_addition_it_next(&addition_it))) {
        UI_CONTROL_ADDITION const * addition_data = ui_data_control_addition_data(addition);
        if (addition_data->type != ui_control_addition_type_res_ref) continue;
        
        plugin_ui_control_frame_create_by_def(
            control,
            (plugin_ui_control_frame_layer_t)addition_data->data.res_ref.layer,
            (plugin_ui_control_frame_usage_t)addition_data->data.res_ref.usage,
            &addition_data->data.res_ref.frame);
    }

    /*加载控件相关的数据 */
    if (control->m_meta->m_load) {
        int r = control->m_meta->m_load(control);
        if (r != 0) {
            CPE_ERROR(module->m_em, "plugin_ui_control_init_from_data: control load fail");
            plugin_ui_control_do_clear(control);
            return r;
        }
    }

    if (control->m_meta->m_on_self_loaded) control->m_meta->m_on_self_loaded(control);

    if (plugin_ui_control_load_childs(control, control->m_template ? control->m_template : control->m_src) != 0) return -1;

    return 0;
}

int plugin_ui_control_set_template(plugin_ui_control_t control, ui_data_control_t template_src) {
    plugin_ui_module_t module = control->m_page->m_env->m_module;
    
    if (ui_data_control_type(template_src) != plugin_ui_control_type(control)) {
        CPE_ERROR(
            module->m_em, "plugin_ui_control_set_template: control type %s and template type %s mismatch!",
            ui_data_control_type_name(control->m_meta->m_type),
            ui_data_control_type_name(ui_data_control_type(template_src)));
        return -1;
    }
            
    while(!TAILQ_EMPTY(&control->m_childs)) {
        plugin_ui_control_free(TAILQ_FIRST(&control->m_childs));
    }

    control->m_template = template_src;

    return plugin_ui_control_do_load_tree(control);
}

int plugin_ui_control_load_tree(plugin_ui_control_t control, ui_data_control_t data_control) {
    plugin_ui_module_t module = control->m_page->m_env->m_module;
    UI_CONTROL const * control_data = ui_data_control_data(data_control);

    if (control->m_meta->m_type != control_data->type) {
        CPE_ERROR(
            module->m_em, "plugin_ui_control_init_from_data: control type %d and %d mismatch!",
            control->m_meta->m_type, control_data->type);
        return -1;
    }
    
    plugin_ui_control_do_clear(control);

    control->m_src = data_control;

    if (control_data->basic.is_link) {
        ui_data_control_t template;
        const char * template_path = plugin_ui_control_msg(control, control_data->basic.link_control_id);

        template = plugin_ui_control_find_template(module, template_path);
        if (template == NULL) {
            CPE_ERROR(module->m_em, "plugin_ui_control_init_from_data: template %s load fail", template_path);
            plugin_ui_control_do_clear(control);
            return -1;
        }

        if (ui_data_control_type(template) == ui_control_type_window) {
            template = ui_data_control_child_find_by_name(template, ui_data_control_name(data_control));
            if (template == NULL) {
                CPE_ERROR(
                    module->m_em, "plugin_ui_control_init_from_data: template %s can`t find sub control %s",
                    template_path,
                    ui_data_control_name(data_control));
                plugin_ui_control_do_clear(control);
                return -1;
            }
            
            if (ui_data_control_type(template) != ui_data_control_type(data_control)) {
                CPE_ERROR(
                    module->m_em, "plugin_ui_control_init_from_data: template %s.%s type is %s, require is %s, msimatch",
                    template_path, ui_data_control_name(data_control),
                    ui_data_control_type_name(ui_data_control_type(template)),
                    ui_data_control_type_name(ui_data_control_type(data_control)));
                plugin_ui_control_do_clear(control);
                return -1;
            }
        }
        else {
            if (ui_data_control_type(template) != ui_data_control_type(data_control)) {
                CPE_ERROR(
                    module->m_em, "plugin_ui_control_init_from_data: template %s type is %s, require is %s, msimatch",
                    template_path,
                    ui_data_control_type_name(ui_data_control_type(template)),
                    ui_data_control_type_name(ui_data_control_type(data_control)));
                plugin_ui_control_do_clear(control);
                return -1;
            }
        }
        
        control->m_template = template;
    }

    return plugin_ui_control_do_load_tree(control);
}

int plugin_ui_control_load_childs(plugin_ui_control_t control, ui_data_control_t data_control) {
    plugin_ui_page_t page = control->m_page;
    struct ui_data_control_it child_data_it;
    ui_data_control_t child_data;
    int rv = 0;

    ui_data_control_childs(&child_data_it, data_control);
    while((child_data = ui_data_control_it_next(&child_data_it))) {
        plugin_ui_control_t child_control = plugin_ui_control_create(page, ui_data_control_type(child_data));
        if (child_control == NULL) {
            rv = -1;
            continue;
        }

        if (plugin_ui_control_load_tree(child_control, child_data) != 0) {
            rv = -1;
            plugin_ui_control_free(child_control);
            continue;
        }

        plugin_ui_control_add_child_tail(control, child_control);
    }
        
    return rv;
}
