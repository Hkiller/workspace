#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_utils_i.h"

uint8_t plugin_ui_control_need_update_cache(plugin_ui_control_t control) {
    return control->m_cache_flag ? 1 : 0;
}

void plugin_ui_control_update_cache(plugin_ui_control_t control, uint16_t cache_flag) {
    uint16_t child_cache_flag = 0;
    uint8_t is_parent_update_pos_abs = cpe_ba_get(&cache_flag, plugin_ui_control_cache_flag_pos_abs) ? 1 : 0;
    plugin_ui_control_t child;
    uint8_t changed = 0;

    cache_flag |= control->m_cache_flag;

    /*颜色 */
    if (cpe_ba_get(&cache_flag, plugin_ui_control_cache_flag_color)) {
        cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_color, cpe_ba_false);
        cpe_ba_set(&child_cache_flag, plugin_ui_control_cache_flag_color, cpe_ba_true);
        changed = 1;

        control->m_render_color = * plugin_ui_control_effective_color(control);
        control->m_render_color.a = control->m_alpha;

        if (control->m_parent) {
            ui_color_inline_mul(&control->m_render_color, &control->m_parent->m_render_color);
        }
    }

    /*角度 */
    if (cpe_ba_get(&cache_flag, plugin_ui_control_cache_flag_angle)) {
        cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_angle, cpe_ba_false);
        cpe_ba_set(&child_cache_flag, plugin_ui_control_cache_flag_angle, cpe_ba_true);
        changed = 1;

		if (ui_vector_3_cmp(&control->m_angle, &UI_VECTOR_3_ZERO) != 0) {
			control->m_render_angle = control->m_angle;
		}
		else {
            control->m_render_angle = control->m_parent ? control->m_parent->m_render_angle : UI_VECTOR_3_ZERO;
		}
    }

    /*缩放 */
    if (cpe_ba_get(&cache_flag, plugin_ui_control_cache_flag_scale)) {
        cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_scale, cpe_ba_false);
        cpe_ba_set(&cache_flag, plugin_ui_control_cache_flag_size, cpe_ba_true);
        cpe_ba_set(&child_cache_flag, plugin_ui_control_cache_flag_scale, cpe_ba_true);
        cpe_ba_set(&child_cache_flag, plugin_ui_control_cache_flag_pos, cpe_ba_true);
        if (ui_vector_2_cmp(&control->m_pivot, &UI_VECTOR_2_ZERO) != 0) {
            cpe_ba_set(&cache_flag, plugin_ui_control_cache_flag_pos_abs, cpe_ba_true);
        }
        changed = 1;

        control->m_render_scale = control->m_scale;
        if (plugin_ui_control_is_float(control)) {
			ui_vector_2_inline_cross_product(&control->m_render_scale, &control->m_float_scale);
        }
        
        /*继承 */
        if (control->m_parent) {
			ui_vector_2_inline_cross_product(&control->m_render_scale, &control->m_parent->m_render_scale);
        }
    }

    /*更新位置 */
    if (cpe_ba_get(&cache_flag, plugin_ui_control_cache_flag_pos)) {
        cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_pos, cpe_ba_false);
        cpe_ba_set(&cache_flag, plugin_ui_control_cache_flag_pos_abs, cpe_ba_true);
        cpe_ba_set(&child_cache_flag, plugin_ui_control_cache_flag_pos_abs, cpe_ba_true); /*告诉子控件已经调度控件位置改动操作 */
        changed = 1;

        if (control->m_parent && !cpe_ba_get(&control->m_parent->m_flag, plugin_ui_control_flag_size_zero)) {
            /*更新相对父控件的坐标 */
            plugin_ui_calc_child(
                &control->m_render_pt_to_p,
                &control->m_render_pt,
                cpe_ba_get(&control->m_flag, plugin_ui_control_flag_accept_ptls),
                &control->m_parent->m_render_sz_ns,
                &control->m_parent->m_editor_sz);

            /* printf( */
            /*     "%s.%s: place at (%f,%f)\n", */
            /*     plugin_ui_page_name(plugin_ui_control_page(control)), */
            /*     plugin_ui_control_name(control), */
            /*     control->m_render_pt_to_p.x, control->m_render_pt_to_p.y); */
        }
    }

    /*更新大小 */
    if (cpe_ba_get(&cache_flag, plugin_ui_control_cache_flag_size)) {
        cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_size, cpe_ba_false);
        cpe_ba_set(&child_cache_flag, plugin_ui_control_cache_flag_size, cpe_ba_true);
        cpe_ba_set(&cache_flag, plugin_ui_control_cache_flag_layout, cpe_ba_true);
        changed = 1;

        if (control->m_parent && !cpe_ba_get(&control->m_parent->m_flag, plugin_ui_control_flag_size_zero)) {
            ui_vector_2 sz;
            
            plugin_ui_calc_child(
                &sz,
                &control->m_render_sz,
                cpe_ba_get(&control->m_flag, plugin_ui_control_flag_accept_szls),
                &control->m_parent->m_render_sz_ns,
                &control->m_parent->m_editor_sz);

            if (sz.x < 1.0f || sz.y < 1.0f) {
                cpe_ba_set(&control->m_flag, plugin_ui_control_flag_size_zero, cpe_ba_true);
            }
            else {
                cpe_ba_set(&control->m_flag, plugin_ui_control_flag_size_zero, cpe_ba_false);
                control->m_render_sz_ns = sz;
            }
            
            plugin_ui_calc_child(
                &control->m_client_real_pd.lt,
                &control->m_client_pd.lt,
                cpe_ba_get(&control->m_flag, plugin_ui_control_flag_accept_ptls),
                &control->m_parent->m_render_sz_ns,
                &control->m_parent->m_editor_sz);

            plugin_ui_calc_child(
                &control->m_client_real_pd.rb,
                &control->m_client_pd.rb,
                cpe_ba_get(&control->m_flag, plugin_ui_control_flag_accept_ptls),
                &control->m_parent->m_render_sz_ns,
                &control->m_parent->m_editor_sz);

            ui_vector_2_cross_product(&control->m_render_sz_abs, &control->m_render_sz_ns, &control->m_render_scale);

            /* printf( */
            /*     "%s.%s: resize to (%f,%f)\n", */
            /*     plugin_ui_page_name(plugin_ui_control_page(control)), */
            /*     plugin_ui_control_name(control), */
            /*     control->m_render_sz_ns.x, control->m_render_sz_ns.y); */
        }
    }

    if (cpe_ba_get(&cache_flag, plugin_ui_control_cache_flag_layout)) {
        cpe_ba_set(&child_cache_flag, plugin_ui_control_cache_flag_pos_abs, cpe_ba_true);
        cpe_ba_set(&cache_flag, plugin_ui_control_cache_flag_pos_abs, cpe_ba_true);
    }
    
    /*更新子控件自生的数据 */
    TAILQ_FOREACH(child, &control->m_childs, m_next_for_parent) {
        plugin_ui_control_update_cache(child, child_cache_flag);
    }

    /*子控件布局 */
    if (cpe_ba_get(&cache_flag, plugin_ui_control_cache_flag_layout)) { /*不需要递归检查 */
        ui_vector_2 client_sz;

        cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_layout, cpe_ba_false);
        changed = 1;

        if (!cpe_ba_get(&control->m_flag, plugin_ui_control_flag_size_zero)) {
            if (control->m_meta->m_layout) {
                uint16_t tmp_flag = 0;

                cpe_ba_set(&tmp_flag, plugin_ui_control_cache_flag_pos_abs, cpe_ba_true);

                control->m_meta->m_layout(control, &client_sz);
                TAILQ_FOREACH(child, &control->m_childs, m_next_for_parent) {
                    plugin_ui_control_update_cache(child, tmp_flag);
                }
            }
            else {
                if (!TAILQ_EMPTY(&control->m_childs)) {
                    plugin_ui_control_basic_layout(control, &client_sz);
                }
            }

            if (control->m_parent) {
                plugin_ui_control_cache_client(control, &client_sz);
            }
        }
    }

    if (is_parent_update_pos_abs) {
        cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_pos_abs, cpe_ba_false);
    }
    else if (cpe_ba_get(&cache_flag, plugin_ui_control_cache_flag_pos_abs)) {
        cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_pos_abs, cpe_ba_false);
        changed = 1;
        plugin_ui_control_cache_render_pt_abs_r(control);
    }

    assert(control->m_cache_flag == 0);

    if (changed) {
        /* CPE_INFO( */
        /*     control->m_page->m_env->m_module->m_em, */
        /*     "plugin_ui_control_update_cache: %s: pt_to_p=(%f,%f), sz=(%f,%f), sz_abs=(%f,%f), scale=(%f,%f), angle=(%f,%f,%f)", */
        /*     plugin_ui_control_name(control), */
        /*     control->m_render_pt_to_p.x, control->m_render_pt_to_p.y, */
        /*     control->m_render_sz_ns.x, control->m_render_sz_ns.y, */
        /*     control->m_render_sz_abs.x, control->m_render_sz_abs.y, */
        /*     control->m_render_scale.x, control->m_render_scale.y, */
        /*     control->m_render_angle.x, control->m_render_angle.y, control->m_render_angle.z); */
    }
}

void plugin_ui_control_check_update_from_root(plugin_ui_control_t control) {
    plugin_ui_control_t update_root = NULL;

    while(control) {
        if (control->m_cache_flag) update_root = control;
        control = control->m_parent;
    }

    if (update_root) {
        plugin_ui_control_update_cache(update_root, 0);
    }
}

uint8_t plugin_ui_control_cache_render_pt_abs(plugin_ui_control_t control) {
    assert(cpe_ba_get(&control->m_cache_flag, plugin_ui_control_cache_flag_pos) == cpe_ba_false);
    assert(cpe_ba_get(&control->m_cache_flag, plugin_ui_control_cache_flag_size) == cpe_ba_false);
    
    cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_pos_abs, cpe_ba_false);

    if (cpe_ba_get(&control->m_parent->m_flag, plugin_ui_control_flag_size_zero)) return -1;

    ui_assert_vector_2_sane(&control->m_parent->m_render_pt_to_p);
    ui_assert_vector_2_sane(&control->m_parent->m_render_scale);
    
    /*计算绝对坐标 */
    control->m_render_pt_abs = control->m_render_pt_to_p;
    ui_vector_2_inline_cross_product(&control->m_render_pt_abs, &control->m_parent->m_render_scale);
            
    /*轴心 */
    if (ui_vector_2_cmp(&control->m_scale, &UI_VECTOR_2_IDENTITY) != 0
        && ui_vector_2_cmp(&control->m_pivot, &UI_VECTOR_2_ZERO) != 0)
    {
        ui_vector_2 sz;
        float ox;
        float oy;

        ui_vector_2_cross_product(&sz, &control->m_render_sz_ns, &control->m_parent->m_render_scale);

        ox = (1.0f - control->m_scale.x) * control->m_pivot.x * sz.x;
        oy = (1.0f - control->m_scale.y) * control->m_pivot.y * sz.y;

        cpe_assert_float_sane(ox);
        cpe_assert_float_sane(oy);
        
        control->m_render_pt_abs.x += ox;
        control->m_render_pt_abs.y += oy;
    }

    /*滚动 */
    control->m_render_pt_abs.x -= control->m_parent ? control->m_parent->m_scroll_pt.x : 0.0f;
    control->m_render_pt_abs.y -= control->m_parent ? control->m_parent->m_scroll_pt.y : 0.0f;

    /*叠加父控件位置 */
    control->m_render_pt_abs.x += control->m_parent->m_render_pt_abs.x;
    control->m_render_pt_abs.y += control->m_parent->m_render_pt_abs.y;

    /*绝对位置变动以后需要调整裁剪框 */
    if (control->m_parent == NULL || !plugin_ui_control_accept_clip(control)) {
        //ui_rect self_rect;

        //self_rect.lt = control->m_render_pt_abs;
        //ui_vector_2_add(&self_rect.rb, &control->m_render_pt_abs, &control->m_render_sz_abs);

        //ui_rect_intersection(&control->m_cliped_rt_abs, &self_rect, &control->m_page->m_control.m_cliped_rt_abs);
        control->m_cliped_rt_abs = control->m_page->m_control.m_cliped_rt_abs;
    }
    else if (plugin_ui_control_parent_clip(control)) {
        control->m_cliped_rt_abs = control->m_parent->m_cliped_rt_abs;
    }
    else {
         //if(plugin_ui_control_accept_clip(control)) {
             ui_rect render_rt;
             /*TODO: 老代码用了父控件的绘制框，应该用自生的绘制框吧 */
             render_rt.lt = control->m_parent->m_render_pt_abs;
             ui_vector_2_add(&render_rt.rb, &control->m_parent->m_render_pt_abs, &control->m_parent->m_render_sz_abs);

             ui_rect_intersection(
                 &control->m_cliped_rt_abs,
                 &render_rt,
                 &control->m_parent->m_cliped_rt_abs);
         //}
        // else{
        //      control->m_cliped_rt_abs = control->m_page->m_control.m_cliped_rt_abs;
        // }
    }

    /* CPE_INFO( */
    /*     control->m_page->m_env->m_module->m_em, */
    /*     "plugin_ui_control_cache_render_pt_abs: %s: pos_abs=(%f,%f), pivot=(%f,%f), clip=(%f,%f)~(%f,%f)", */
    /*     plugin_ui_control_name(control), */
    /*     control->m_render_pt_abs.x, control->m_render_pt_abs.y, */
    /*     control->m_cliped_rt_abs.lt.x, control->m_cliped_rt_abs.lt.y, control->m_cliped_rt_abs.rb.x, control->m_cliped_rt_abs.rb.y); */
    
    return 0;
}

void plugin_ui_control_cache_render_pt_abs_r(plugin_ui_control_t control) {
    plugin_ui_control_t child;

    if (control->m_parent) {
        if (plugin_ui_control_cache_render_pt_abs(control) != 0) return;
    }

    TAILQ_FOREACH(child, &control->m_childs, m_next_for_parent) {
        plugin_ui_control_cache_render_pt_abs_r(child);
    }
}

void plugin_ui_control_cache_client(plugin_ui_control_t control, ui_vector_2_t client_sz) {
    if (cpe_ba_get(&control->m_parent->m_flag, plugin_ui_control_flag_size_zero)) {
        return;
    }

    plugin_ui_calc_child(
        &control->m_client_real_sz,
        &control->m_client_sz,
        cpe_ba_get(&control->m_flag, plugin_ui_control_flag_accept_szls),
        &control->m_parent->m_render_sz_ns,
        &control->m_parent->m_editor_sz);

    if (client_sz->x > control->m_client_real_sz.x) control->m_client_real_sz.x = client_sz->x;
    if (client_sz->y > control->m_client_real_sz.y) control->m_client_real_sz.y = client_sz->y;
}
