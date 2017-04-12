#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "render/utils/ui_vector_2.h"
#include "render/model/ui_data_layout.h"
#include "render/model/ui_data_utils.h"
#include "plugin/basicanim/plugin_basicanim_color.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_control_attr_meta_i.h"
#include "plugin_ui_control_calc_i.h"
#include "plugin_ui_control_frame_i.h"
#include "plugin_ui_aspect_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_utils_i.h"
#include "plugin_ui_touch_track_i.h"
#include "plugin_ui_control_category_i.h"

ui_color const * plugin_ui_control_effective_color(plugin_ui_control_t control) {
    return plugin_ui_control_enable(control) ? &control->m_color : &control->m_gray_color;
}

ui_color const * plugin_ui_control_color(plugin_ui_control_t control) {
    return &control->m_color;
}

void plugin_ui_control_set_color(plugin_ui_control_t control, ui_color const * color) {
    cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_color, cpe_ba_true);
    control->m_color = *color;
}

ui_color const * plugin_ui_control_gray_color(plugin_ui_control_t control) {
    return &control->m_gray_color;
}

void plugin_ui_control_set_gray_color(plugin_ui_control_t control, ui_color const * gray_color) {
    cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_color, cpe_ba_true);
    control->m_gray_color = *gray_color;
}

float plugin_ui_control_alpha(plugin_ui_control_t control) {
    return control->m_alpha;
}

void plugin_ui_control_set_alpha(plugin_ui_control_t control, float alpha) {
    if(alpha < 0.0f) alpha = 0.0f;
    if(alpha > 1.0f) alpha = 1.0f;
    if (control->m_alpha != alpha) {
        cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_color, cpe_ba_true);
        control->m_alpha = alpha;
    }
}

uint8_t plugin_ui_control_align_vert(plugin_ui_control_t control) {
    return control->m_align_vert;
}

void plugin_ui_control_set_align_vert(plugin_ui_control_t control, uint8_t align_vert) {
    if (control->m_align_vert == align_vert) return;

    control->m_align_vert = align_vert;
}

uint8_t plugin_ui_control_align_horz(plugin_ui_control_t control) {
    return control->m_align_horz;
}

void plugin_ui_control_set_align_horz(plugin_ui_control_t control, uint8_t align_horz) {
    if (control->m_align_horz == align_horz) return;

    control->m_align_horz = align_horz;
    /*
    if (control->m_parent) {
        cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_layout, cpe_ba_true);
	}
     */
}

ui_vector_2_t plugin_ui_control_editor_pt(plugin_ui_control_t control) {
    return &control->m_editor_pt;
}

ui_vector_2_t plugin_ui_control_editor_sz(plugin_ui_control_t control) {
    return &control->m_editor_sz;
}

ui_rect_t plugin_ui_control_editor_pd(plugin_ui_control_t control) {
    assert(cpe_ba_get(&control->m_cache_flag, plugin_ui_control_cache_flag_size) == cpe_ba_false);
    return &control->m_editor_pd;
}

ui_vector_2_t plugin_ui_control_real_pt_to_p(plugin_ui_control_t control) {
    assert(cpe_ba_get(&control->m_cache_flag, plugin_ui_control_cache_flag_pos) == cpe_ba_false);
    return &control->m_render_pt_to_p;
}

ui_vector_2 plugin_ui_control_real_pt_to_p_no_screen_adj(plugin_ui_control_t control) {
    ui_vector_2 pt = * plugin_ui_control_real_pt_to_p(control);
    pt.x /= control->m_page->m_env->m_screen_adj.x;
    pt.y /= control->m_page->m_env->m_screen_adj.y;
    return pt;
}

ui_vector_2_t plugin_ui_control_real_pt_abs(plugin_ui_control_t control) {
    assert(cpe_ba_get(&control->m_cache_flag, plugin_ui_control_cache_flag_pos_abs) == cpe_ba_false);
    return &control->m_render_pt_abs;
}

ui_vector_2_t plugin_ui_control_real_sz_no_scale(plugin_ui_control_t control) {
    assert(cpe_ba_get(&control->m_cache_flag, plugin_ui_control_cache_flag_size) == cpe_ba_false);
    if (cpe_ba_get(&control->m_flag, plugin_ui_control_flag_size_zero)) {
        return &UI_VECTOR_2_ZERO;
    }
    else {
        return &control->m_render_sz_ns;
    }
}

ui_vector_2 plugin_ui_control_calc_child_real_sz_no_scale(plugin_ui_control_t control, ui_data_control_t child_data) {
    ui_vector_2 r;
    UI_CONTROL const * d;
    UI_UNIT_VECTOR_2 rule;

    assert(cpe_ba_get(&control->m_cache_flag, plugin_ui_control_cache_flag_size) == cpe_ba_false);

    d = ui_data_control_data(child_data);

    assert(control->m_render_sz_ns.x >= 1.0f && control->m_render_sz_ns.y >= 1.0f);

    //fixed by beik
    if(!d->basic.accept_sz_ls){
        float min;
        rule.x.b = 0;
        rule.y.b = 0;
        min = cpe_min(control->m_editor_sz.x, control->m_editor_sz.y);
        rule.x.k = d->basic.editor_sz.value[0] / min;
        rule.y.k = d->basic.editor_sz.value[1] / min;
    }else {
        rule.x.b = 0;
        rule.y.b = 0;
        rule.x.k = d->basic.editor_sz.value[0] / control->m_editor_sz.x;
        rule.y.k = d->basic.editor_sz.value[1] / control->m_editor_sz.y;
    }

    plugin_ui_calc_child(
        &r,
        &rule/*&d->basic.render_sz*/, d->basic.accept_sz_ls,
        &control->m_render_sz_ns, &control->m_editor_sz);

    return r;
}

ui_vector_2_t plugin_ui_control_real_sz_abs(plugin_ui_control_t control) {
    return &control->m_render_sz_abs;
}

ui_rect plugin_ui_control_real_rt_no_scale(plugin_ui_control_t control) {
    ui_rect r;
    
    assert(cpe_ba_get(&control->m_cache_flag, plugin_ui_control_cache_flag_scale) == cpe_ba_false);
    assert(cpe_ba_get(&control->m_cache_flag, plugin_ui_control_cache_flag_size) == cpe_ba_false);
    assert(cpe_ba_get(&control->m_cache_flag, plugin_ui_control_cache_flag_pos_abs) == cpe_ba_false);

    r.lt = control->m_render_pt_abs;

    if (cpe_ba_get(&control->m_flag, plugin_ui_control_flag_size_zero)) {
        r.rb = r.lt;
    }
    else {
        r.rb.x = control->m_render_pt_abs.x + control->m_render_sz_ns.x;
        r.rb.y = control->m_render_pt_abs.y + control->m_render_sz_ns.y;
    }
    
	return r;
}

ui_rect plugin_ui_control_real_rt_abs(plugin_ui_control_t control) {
    ui_rect r;
    
    assert(cpe_ba_get(&control->m_cache_flag, plugin_ui_control_cache_flag_scale) == cpe_ba_false);
    assert(cpe_ba_get(&control->m_cache_flag, plugin_ui_control_cache_flag_size) == cpe_ba_false);
    assert(cpe_ba_get(&control->m_cache_flag, plugin_ui_control_cache_flag_pos_abs) == cpe_ba_false);

    r.lt = control->m_render_pt_abs;

    if (cpe_ba_get(&control->m_flag, plugin_ui_control_flag_size_zero)) {
        r.rb = r.lt;
    }
    else {
        r.rb.x = control->m_render_pt_abs.x + control->m_render_sz_ns.x * control->m_render_scale.x;
        r.rb.y = control->m_render_pt_abs.y + control->m_render_sz_ns.y * control->m_render_scale.y;
    }
    
	return r;
}

ui_rect plugin_ui_control_real_inner_abs(plugin_ui_control_t control) {
    ui_rect r;
    
    assert(cpe_ba_get(&control->m_cache_flag, plugin_ui_control_cache_flag_scale) == cpe_ba_false);
    assert(cpe_ba_get(&control->m_cache_flag, plugin_ui_control_cache_flag_size) == cpe_ba_false);
    assert(cpe_ba_get(&control->m_cache_flag, plugin_ui_control_cache_flag_pos_abs) == cpe_ba_false);

    if (cpe_ba_get(&control->m_flag, plugin_ui_control_flag_size_zero)) {
        r.lt = r.rb = control->m_render_pt_abs;
    }
    else {
        r.lt.x = control->m_client_real_pd.lt.x;
        r.lt.y = control->m_client_real_pd.lt.y;
        r.rb.x = control->m_render_sz_ns.x - control->m_client_real_pd.rb.x;
        r.rb.y = control->m_render_sz_ns.y - control->m_client_real_pd.rb.y;

    
        r.lt.x = (int32_t)(r.lt.x * control->m_render_scale.x);
        r.lt.y = (int32_t)(r.lt.y * control->m_render_scale.y);
        r.rb.x = (int32_t)(r.rb.x * control->m_render_scale.x);
        r.rb.y = (int32_t)(r.rb.y * control->m_render_scale.y);

        r.lt.x += control->m_render_pt_abs.x;
        r.lt.y += control->m_render_pt_abs.y;
        r.rb.x += control->m_render_pt_abs.x;
        r.rb.y += control->m_render_pt_abs.y;
    }
    
	return r;
}

ui_vector_2_t plugin_ui_control_pivot(plugin_ui_control_t control) {
    return &control->m_pivot;
}

void plugin_ui_control_set_pivot(plugin_ui_control_t control, ui_vector_2_t pivot) {
    cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_pos, cpe_ba_true);
    control->m_pivot = *pivot;
    assert(0);
}

ui_vector_3_t plugin_ui_control_angle(plugin_ui_control_t control) {
    return &control->m_angle;
}

void plugin_ui_control_set_angle(plugin_ui_control_t control, ui_vector_3_t angle) {
    cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_angle, cpe_ba_true);
    control->m_angle = *angle;
}

ui_vector_2_t plugin_ui_control_scale(plugin_ui_control_t control) {
    return &control->m_scale;
}

void plugin_ui_control_set_scale(plugin_ui_control_t control, ui_vector_2_t scale) {
    if (ui_vector_2_cmp(&control->m_scale, scale) != 0) {
        cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_scale, cpe_ba_true);
        control->m_scale = *scale;
    }
}

ui_vector_2_t plugin_ui_control_all_frame_pos(plugin_ui_control_t control) {
    return &control->m_all_frame_pt;
}

void plugin_ui_control_set_all_frame_pos(plugin_ui_control_t control, ui_vector_2_t pos) {
    control->m_all_frame_pt = *pos;
}
    
ui_vector_2_t plugin_ui_control_all_frame_scale(plugin_ui_control_t control) {
    return &control->m_all_frame_scale;
}

void plugin_ui_control_set_all_frame_scale(plugin_ui_control_t control, ui_vector_2_t scale) {
    control->m_all_frame_scale = *scale;
}

uint8_t plugin_ui_control_accept_global_resize(plugin_ui_control_t control) {
    return cpe_ba_get(&control->m_flag, plugin_ui_control_flag_disable_global_resize) ? 0 : 1;
}

void plugin_ui_control_set_accept_global_resize(plugin_ui_control_t control, uint8_t accept) {
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_disable_global_resize, accept ? cpe_ba_false : cpe_ba_true);
}

uint8_t plugin_ui_control_visible(plugin_ui_control_t control) {
    return cpe_ba_get(&control->m_flag, plugin_ui_control_flag_visible) ? 1 : 0;
}

ui_vector_2 plugin_ui_control_pt_world_to_local(plugin_ui_control_t control, ui_vector_2_t pt) {
    ui_vector_2 r = *pt;
    ui_vector_2_t real_pt = plugin_ui_control_real_pt_abs(control);
    r.x -= real_pt->x;
    r.y -= real_pt->y;
    r.x /= control->m_scale.x;
    r.y /= control->m_scale.y;
    return r;
}

void plugin_ui_control_set_visible(plugin_ui_control_t control, uint8_t visible) {
    visible = visible ? 1 : 0;
	if (control->m_is_free) return;

    if (control == &control->m_page->m_control) {
        if (visible) {
            plugin_ui_page_show_in_current_state(control->m_page, NULL, NULL, NULL, NULL);
        }
        else {
            plugin_ui_page_hide(control->m_page);
        }
    }
    else {
        if (plugin_ui_control_visible(control) == visible) return;

        cpe_ba_set(&control->m_flag, plugin_ui_control_flag_visible, visible ? cpe_ba_true : cpe_ba_false);

		if (visible) {
            plugin_ui_control_dispatch_event(control, control, plugin_ui_event_show, plugin_ui_event_dispatch_to_self_and_parent);
        }
		else {
            plugin_ui_control_dispatch_event(control, control, plugin_ui_event_hide, plugin_ui_event_dispatch_to_self_and_parent);
        }
    }
}

plugin_ui_control_t plugin_ui_control_hide_by(plugin_ui_control_t control) {
    while(control) {
        if (!cpe_ba_get(&control->m_flag, plugin_ui_control_flag_visible)) return control;
        control = control->m_parent;
    }

    return NULL;
}

uint8_t plugin_ui_control_enable(plugin_ui_control_t control) {
    return cpe_ba_get(&control->m_flag, plugin_ui_control_flag_enable) ? 1 : 0;
}

void plugin_ui_control_set_enable(plugin_ui_control_t control, uint8_t enable) {
    if (plugin_ui_control_enable(control) == enable) return;

    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_enable, enable ? cpe_ba_true : cpe_ba_false);
    cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_color, cpe_ba_true);
}

uint8_t plugin_ui_control_is_float(plugin_ui_control_t control) {
    return cpe_ba_get(&control->m_render_layers, plugin_ui_control_frame_layer_float) ? 1 : 0;
}

void plugin_ui_control_set_float(plugin_ui_control_t control, uint8_t is_float) {
    is_float = is_float ? 1 : 0;
    if (is_float == plugin_ui_control_is_float(control)) return;
    
    cpe_ba_set(&control->m_render_layers, plugin_ui_control_frame_layer_float, is_float ? cpe_ba_true : cpe_ba_false);

    if (ui_vector_2_cmp(&control->m_float_scale, &UI_VECTOR_2_IDENTITY) != 0) {
        cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_scale, cpe_ba_true);
    }
}

uint8_t plugin_ui_control_force_clicp(plugin_ui_control_t control) {
    return cpe_ba_get(&control->m_flag, plugin_ui_control_flag_force_clicp) ? 1 : 0;
}

void plugin_ui_control_set_force_clicp(plugin_ui_control_t control, uint8_t force_clicp) {
    if (plugin_ui_control_force_clicp(control) == force_clicp) return;

    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_force_clicp, force_clicp ? cpe_ba_true : cpe_ba_false);
    cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_color, cpe_ba_true);
}

uint8_t plugin_ui_control_accept_click(plugin_ui_control_t control) {
    return cpe_ba_get(&control->m_flag, plugin_ui_control_flag_accept_click) ? 1 : 0;
}

void plugin_ui_control_set_accept_click(plugin_ui_control_t control, uint8_t accept_click) {
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_accept_click, accept_click ? cpe_ba_true : cpe_ba_false);
}

uint8_t plugin_ui_control_accept_global_down_color(plugin_ui_control_t control) {
    return cpe_ba_get(&control->m_flag, plugin_ui_control_flag_accept_global_disable_color) ? 1 : 0;
}

void plugin_ui_control_set_accept_global_down_color(plugin_ui_control_t control, uint8_t accept_global_down_color) {
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_accept_global_disable_color, accept_global_down_color ? cpe_ba_true : cpe_ba_false);
}

uint8_t plugin_ui_control_accept_float(plugin_ui_control_t control) {
    return cpe_ba_get(&control->m_flag, plugin_ui_control_flag_accept_float) ? 1 : 0;
}

void plugin_ui_control_set_accept_float(plugin_ui_control_t control, uint8_t accept_float) {
    if (plugin_ui_control_accept_float(control) == accept_float) return;
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_accept_float, accept_float ? cpe_ba_true : cpe_ba_false);
}

uint8_t plugin_ui_control_draw_align(plugin_ui_control_t control) {
    return control->m_draw_align;
}

void plugin_ui_control_set_draw_align(plugin_ui_control_t control, uint8_t draw_align) {
    control->m_draw_align = draw_align;
}
    
plugin_ui_control_frame_t plugin_ui_control_draw_color(plugin_ui_control_t control) {
    plugin_ui_control_frame_t frame;
    plugin_ui_aspect_t lock_aspect = plugin_ui_control_lock_aspect(control);
    
    TAILQ_FOREACH(frame, &control->m_frames, m_next) {
        if (frame->m_layer != plugin_ui_control_frame_layer_back) continue;
        if (!plugin_ui_aspect_control_frame_is_in(lock_aspect, frame)) continue;
        if (strcmp(plugin_ui_control_frame_obj_type_name(frame), "color") != 0) continue;
        return frame;
    }

    return NULL;
}

plugin_ui_control_frame_t
plugin_ui_control_set_draw_color(plugin_ui_control_t control, uint8_t draw_color) {
    plugin_ui_control_frame_t frame = plugin_ui_control_draw_color(control);
    
    if (draw_color) {
        if (frame == NULL) {
            frame = plugin_ui_control_frame_create_by_type(control, plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_normal, "color", NULL);
            if (frame == NULL) return NULL;

            plugin_ui_control_frame_set_sync_size(frame, 1);
            plugin_ui_control_frame_set_base_pos(frame, ui_pos_policy_top_left);
            plugin_ui_control_frame_set_sync_size(frame, 1);
            plugin_ui_aspect_control_frame_add(plugin_ui_control_lock_aspect(control), frame, 1);
        }
    }
    else {
        if (frame) {
            plugin_ui_control_frame_free(frame);
            frame = NULL;
        }
    }

    return frame;
}

uint8_t plugin_ui_control_frame_changed(plugin_ui_control_t control) {
    return cpe_ba_get(&control->m_flag, plugin_ui_control_flag_frame_updated) ? 1 : 0;
}

uint8_t plugin_ui_control_draw_frame(plugin_ui_control_t control) {
    return cpe_ba_get(&control->m_render_usables, plugin_ui_control_frame_usage_normal) ? 1 : 0;
}

void plugin_ui_control_set_draw_frame(plugin_ui_control_t control, uint8_t draw_frame) {
    cpe_ba_set(&control->m_render_usables, plugin_ui_control_frame_usage_normal, draw_frame ? cpe_ba_true : cpe_ba_false);
}

uint8_t plugin_ui_control_draw_inner(plugin_ui_control_t control) {
    return cpe_ba_get(&control->m_flag, plugin_ui_control_flag_draw_inner) ? 1 : 0;
}

void plugin_ui_control_set_draw_inner(plugin_ui_control_t control, uint8_t draw_inner) {
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_draw_inner, draw_inner ? cpe_ba_true : cpe_ba_false);
}

uint8_t plugin_ui_control_accept_ptls(plugin_ui_control_t control) {
    return cpe_ba_get(&control->m_flag, plugin_ui_control_flag_accept_ptls) ? 1 : 0;
}

void plugin_ui_control_set_accept_ptls(plugin_ui_control_t control, uint8_t accept_ptls) {
    cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_pos, cpe_ba_true);
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_accept_ptls, accept_ptls ? cpe_ba_true : cpe_ba_false);

    assert(control->m_parent);
    cpe_ba_set(&control->m_parent->m_cache_flag, plugin_ui_control_cache_flag_layout, cpe_ba_true);
}    

uint8_t plugin_ui_control_accept_szls(plugin_ui_control_t control) {
    return cpe_ba_get(&control->m_flag, plugin_ui_control_flag_accept_szls) ? 1 : 0;
}

void plugin_ui_control_set_accept_szls(plugin_ui_control_t control, uint8_t accept_szls) {
    cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_pos, cpe_ba_true);
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_accept_szls, accept_szls ? cpe_ba_true : cpe_ba_false);

    assert(control->m_parent);
    cpe_ba_set(&control->m_parent->m_cache_flag, plugin_ui_control_cache_flag_layout, cpe_ba_true);
}

uint8_t plugin_ui_control_accept_move(plugin_ui_control_t control) {
    return cpe_ba_get(&control->m_flag, plugin_ui_control_flag_accept_move) ? 1 : 0;
}

void plugin_ui_control_set_accept_move(plugin_ui_control_t control, uint8_t accept_move) {
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_accept_move, accept_move ? cpe_ba_true : cpe_ba_false);
}    

uint8_t plugin_ui_control_accept_clip(plugin_ui_control_t control) {
    return cpe_ba_get(&control->m_flag, plugin_ui_control_flag_accept_clip) ? 1 : 0;
}

void plugin_ui_control_set_accept_clip(plugin_ui_control_t control, uint8_t accept_clip) {
    cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_pos, cpe_ba_true);
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_accept_clip, accept_clip ? cpe_ba_true : cpe_ba_false);
}    

uint8_t plugin_ui_control_parent_clip(plugin_ui_control_t control) {
    return cpe_ba_get(&control->m_flag, plugin_ui_control_flag_parent_clip) ? 1 : 0;
}

void plugin_ui_control_set_parent_clip(plugin_ui_control_t control, uint8_t parent_clip) {
    cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_pos, cpe_ba_true);
    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_parent_clip, parent_clip ? cpe_ba_true : cpe_ba_false);
}

ui_vector_2_t plugin_ui_control_scroll(plugin_ui_control_t control) {
    return &control->m_scroll_pt;
}

void plugin_ui_control_set_scroll(plugin_ui_control_t control, ui_vector_2_t scroll) {
    uint8_t v_changed = 0;
    uint8_t h_changed = 0;
    
    if (control->m_scroll_pt.x != scroll->x) {
        h_changed = 1;
        control->m_scroll_pt.x = scroll->x;
    }

    if (control->m_scroll_pt.y != scroll->y) {
        v_changed = 1;
        control->m_scroll_pt.y = scroll->y;
    }

    if (v_changed || h_changed) {
        plugin_ui_control_t child;
        TAILQ_FOREACH(child, &control->m_childs, m_next_for_parent) {
            cpe_ba_set(&child->m_cache_flag, plugin_ui_control_cache_flag_pos_abs, cpe_ba_true);
        }
    }

    if (v_changed) {
        plugin_ui_control_dispatch_event(
            control, control,
            plugin_ui_event_vscroll_changed, plugin_ui_event_dispatch_to_self_and_parent);
    }

    if (h_changed) {
        plugin_ui_control_dispatch_event(
            control, control,
            plugin_ui_event_hscroll_changed, plugin_ui_event_dispatch_to_self_and_parent);
    }
}

void plugin_ui_control_set_scroll_x(plugin_ui_control_t control, float scroll_x) {
    plugin_ui_control_t child;
    
    if (control->m_scroll_pt.x == scroll_x) return;
    
    TAILQ_FOREACH(child, &control->m_childs, m_next_for_parent) {
        cpe_ba_set(&child->m_cache_flag, plugin_ui_control_cache_flag_pos_abs, cpe_ba_true);
    }
    control->m_scroll_pt.x = scroll_x;

    plugin_ui_control_dispatch_event(
        control, control,
        plugin_ui_event_hscroll_changed, plugin_ui_event_dispatch_to_self_and_parent);
}

void plugin_ui_control_set_scroll_y(plugin_ui_control_t control, float scroll_y) {
    plugin_ui_control_t child;
    
    if (control->m_scroll_pt.y == scroll_y) return;
    
    TAILQ_FOREACH(child, &control->m_childs, m_next_for_parent) {
        cpe_ba_set(&child->m_cache_flag, plugin_ui_control_cache_flag_pos_abs, cpe_ba_true);
    }
    control->m_scroll_pt.y = scroll_y;

    plugin_ui_control_dispatch_event(
        control, control,
        plugin_ui_event_vscroll_changed, plugin_ui_event_dispatch_to_self_and_parent);
}

UI_UNIT_VECTOR_2 const * plugin_ui_control_render_pt(plugin_ui_control_t control) {
    return &control->m_render_pt;
}

void plugin_ui_control_set_render_pt(plugin_ui_control_t control, UI_UNIT_VECTOR_2 const * pt) {
    cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_pos, cpe_ba_true);
    ui_assert_unit_vector_2_sane(pt);
    control->m_render_pt = *pt;
}

void plugin_ui_control_set_render_pt_x(plugin_ui_control_t control, UI_UNIT const * pt_x) {
    cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_pos, cpe_ba_true);
    control->m_render_pt.x = *pt_x;
}

void plugin_ui_control_set_render_pt_y(plugin_ui_control_t control, UI_UNIT const * pt_y) {
    cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_pos, cpe_ba_true);
    control->m_render_pt.y = *pt_y;
}

void plugin_ui_control_set_render_pt_by_real(plugin_ui_control_t control, ui_vector_2_t real_pt) {
    UI_UNIT_VECTOR_2 new_value;
    struct ui_vector_2 pt;
    
    ui_assert_vector_2_sane(real_pt);
    assert(control->m_parent);

	plugin_ui_control_check_update_from_root(control);
    assert(cpe_ba_get(&control->m_parent->m_cache_flag, plugin_ui_control_cache_flag_size) == cpe_ba_false);
    assert(cpe_ba_get(&control->m_parent->m_cache_flag, plugin_ui_control_cache_flag_pos_abs) == cpe_ba_false);

    if (cpe_ba_get(&control->m_parent->m_flag, plugin_ui_control_flag_size_zero)) return;

    pt = *real_pt;
    
    /*轴心 */
    if (ui_vector_2_cmp(&control->m_scale, &UI_VECTOR_2_IDENTITY) != 0 && ui_vector_2_cmp(&control->m_pivot, &UI_VECTOR_2_ZERO) != 0) {
        ui_vector_2 sz;
        float ox;
        float oy;

        ui_vector_2_cross_product(&sz, &control->m_render_sz_ns, &control->m_parent->m_render_scale);

        ox = (1.0f - control->m_scale.x) * control->m_pivot.x * sz.x;
        oy = (1.0f - control->m_scale.y) * control->m_pivot.y * sz.y;

        cpe_assert_float_sane(ox);
        cpe_assert_float_sane(oy);
        
        pt.x -= ox;
        pt.y -= oy;
    }

    ui_assert_vector_2_sane(&control->m_parent->m_render_scale);

    /*计算绝对坐标 */
    pt.x /= control->m_parent->m_render_scale.x;
    pt.y /= control->m_parent->m_render_scale.y;

    /**/
    plugin_ui_calc_child_unit(
        &new_value,
        plugin_ui_control_accept_ptls(control),
        &control->m_parent->m_render_sz_ns,
        &control->m_parent->m_editor_sz,
        &pt);
    ui_assert_unit_vector_2_sane(&new_value);
    
    if (memcmp(&new_value, &control->m_render_pt, sizeof(new_value)) != 0) {
        cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_pos, cpe_ba_true);
        control->m_render_pt = new_value;
    }
}

void plugin_ui_control_set_render_pt_x_by_real(plugin_ui_control_t control, float x) {
    UI_UNIT y = control->m_render_pt.y;
    ui_vector_2 real_pt = UI_VECTOR_2_INITLIZER(x, control->m_render_pt_to_p.y);
    plugin_ui_control_set_render_pt_by_real(control, &real_pt);
    control->m_render_pt.y = y;
}

void plugin_ui_control_set_render_pt_y_by_real(plugin_ui_control_t control, float y) {
    UI_UNIT x = control->m_render_pt.x;
    ui_vector_2 real_pt = UI_VECTOR_2_INITLIZER(control->m_render_pt_to_p.x, y);
    plugin_ui_control_set_render_pt_by_real(control, &real_pt);
    control->m_render_pt.x = x;
}

void plugin_ui_control_set_render_pt_abs(plugin_ui_control_t control, ui_vector_2_t real_pt) {
    ui_vector_2 pt_to_p;
    
    assert(control->m_parent);
    assert(cpe_ba_get(&control->m_parent->m_cache_flag, plugin_ui_control_cache_flag_size) == cpe_ba_false);
    assert(cpe_ba_get(&control->m_parent->m_cache_flag, plugin_ui_control_cache_flag_pos_abs) == cpe_ba_false);

    pt_to_p.x = real_pt->x - control->m_parent->m_render_pt_abs.x;
    pt_to_p.y = real_pt->y - control->m_parent->m_render_pt_abs.y;

    plugin_ui_control_set_render_pt_by_real(control, &pt_to_p);
}

void plugin_ui_control_set_render_sz(plugin_ui_control_t control, UI_UNIT_VECTOR_2 const * sz) {
    cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_size, cpe_ba_true);
    control->m_render_sz = *sz;
}

void plugin_ui_control_set_render_sz_w(plugin_ui_control_t control, UI_UNIT const * sz_w) {
    cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_size, cpe_ba_true);
    control->m_render_sz.x = *sz_w;
}

void plugin_ui_control_set_render_sz_h(plugin_ui_control_t control, UI_UNIT const * sz_h) {
    cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_size, cpe_ba_true);
    control->m_render_sz.y = *sz_h;
}

void plugin_ui_control_set_render_sz_by_real(plugin_ui_control_t control, ui_vector_2_t real_sz) {
    if (real_sz->x < 1.0f || real_sz->y < 1.0f) {
        cpe_ba_set(&control->m_flag, plugin_ui_control_flag_size_zero, cpe_ba_true);
    }
    else {
        cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_size, cpe_ba_true);
        cpe_ba_set(&control->m_flag, plugin_ui_control_flag_size_zero, cpe_ba_false);

        assert(control->m_parent);
        assert(cpe_ba_get(&control->m_parent->m_cache_flag, plugin_ui_control_cache_flag_size) == cpe_ba_false);
    
        plugin_ui_calc_child_unit(
            &control->m_render_sz,
            plugin_ui_control_accept_szls(control),
            &control->m_parent->m_render_sz_ns,
            &control->m_parent->m_editor_sz,
            real_sz);
    }
}

void plugin_ui_control_set_render_sz_w_by_real(plugin_ui_control_t control, float real_w) {
    UI_UNIT h = control->m_render_sz.y;
    ui_vector_2 real_sz = { { { real_w, control->m_render_sz_ns.y } } };
    plugin_ui_control_set_render_sz_by_real(control, &real_sz);
    control->m_render_sz.y = h;
}

void plugin_ui_control_set_render_sz_h_by_real(plugin_ui_control_t control, float real_h) {
    UI_UNIT w = control->m_render_sz.x;
    ui_vector_2 real_sz = { { { control->m_render_sz_ns.x, real_h } } };
    plugin_ui_control_set_render_sz_by_real(control, &real_sz);
    control->m_render_sz.x = w;
}

int plugin_ui_control_set_render_sz_by_frames(plugin_ui_control_t control) {
    plugin_ui_env_t env = control->m_page->m_env;
    ui_vector_2 sz = UI_VECTOR_2_ZERO;
    plugin_ui_control_frame_t frame;

    TAILQ_FOREACH(frame, &control->m_frames, m_next) {
        ui_vector_2 frame_rb;
        
        if (frame->m_usage != plugin_ui_control_frame_usage_normal) continue;

        frame_rb.x = (frame->m_offset.x + frame->m_base_size.x) * env->m_screen_adj.x;
        frame_rb.y = (frame->m_offset.y + frame->m_base_size.y) * env->m_screen_adj.y;

        if (frame_rb.x > sz.x) sz.x = frame_rb.x;
        if (frame_rb.y > sz.y) sz.y = frame_rb.y;
    }

    if (sz.x > 0.0f && sz.y > 0.0f) {
        plugin_ui_control_set_render_sz_by_real(control, &sz);
        return 0;
    }
    else {
        return -1;
    }
}

ui_vector_2_t plugin_ui_control_client_real_sz(plugin_ui_control_t control) {
    assert(cpe_ba_get(&control->m_cache_flag, plugin_ui_control_cache_flag_size) == cpe_ba_false);

    if (cpe_ba_get(&control->m_parent->m_flag, plugin_ui_control_flag_size_zero)) {
        return &UI_VECTOR_2_ZERO;
    }
    else {
        return &control->m_client_real_sz;
    }
}

void plugin_ui_control_set_client_real_sz(plugin_ui_control_t control, ui_vector_2_t real_sz) {
    cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_size, cpe_ba_true);

    if (cpe_ba_get(&control->m_parent->m_flag, plugin_ui_control_flag_size_zero)) return;
    
    if (control->m_parent) {
        plugin_ui_calc_child_unit(
            &control->m_client_sz,
            plugin_ui_control_accept_szls(control),
            &control->m_parent->m_render_sz_ns,
            &control->m_parent->m_editor_sz,
            real_sz);
    }
    else {
        assert(0);
    }
}

void plugin_ui_control_set_cache_flag_layout(plugin_ui_control_t control, uint8_t layout) {
    cpe_ba_set(&control->m_cache_flag, plugin_ui_control_cache_flag_layout, layout ? cpe_ba_true : cpe_ba_false);
}

ui_rect_t plugin_ui_control_client_real_pd(plugin_ui_control_t control) {
    assert(cpe_ba_get(&control->m_cache_flag, plugin_ui_control_cache_flag_size) == cpe_ba_false);
    return &control->m_client_real_pd;
}

ui_color_t plugin_ui_control_render_color(plugin_ui_control_t control) {
    assert(cpe_ba_get(&control->m_cache_flag, plugin_ui_control_cache_flag_color) == cpe_ba_false);
    return &control->m_render_color;
}

ui_vector_2_t plugin_ui_control_render_scale(plugin_ui_control_t control) {
    assert(cpe_ba_get(&control->m_cache_flag, plugin_ui_control_cache_flag_scale) == cpe_ba_false);
    return &control->m_render_scale;
}

ui_vector_3_t plugin_ui_control_render_angle(plugin_ui_control_t control) {
    return &control->m_render_angle;
}

void plugin_ui_control_set_usage_render(
    plugin_ui_control_t control, plugin_ui_control_frame_usage_t usage, uint8_t is_enable)
{
    assert((uint8_t)usage < sizeof(control->m_render_usables) * 8);
    cpe_ba_set(&control->m_render_usables, usage, is_enable ? cpe_ba_true : cpe_ba_false);
}

ui_rect_t plugin_ui_control_cliped_rt_abs(plugin_ui_control_t control) {
    assert(cpe_ba_get(&control->m_cache_flag, plugin_ui_control_cache_flag_pos_abs) == cpe_ba_false);
    return &control->m_cliped_rt_abs;
}

uint8_t plugin_ui_control_contain_test(plugin_ui_control_t control, ui_vector_2_t pt) {
    ui_rect check;
    ui_rect abs;
    
	/*旋转不可点击  */
	/* if (GetRenderRealAngle() != RVector3::Zero) */
	/* 	return false; */

    if (control->m_cache_flag) plugin_ui_control_check_update_from_root(control);

	/*考虑到客户区大小可能不同于实际绘制大小  */
	/*所以以绘制区域为判定区域  */
    abs = plugin_ui_control_real_rt_abs(control);
    check = control->m_cliped_rt_abs;
    ui_rect_inline_intersection(&check, &abs);

    /* printf("xxxxx: check=(%f,%f)-(%f,%f), pt=(%f,%f)\n", */
    /*        check.lt.x, check.lt.y, check.rb.x, check.rb.y, */
    /*        pt->x, pt->y); */

    return ui_rect_is_valid(&check)
        ? ui_rect_is_contain_pt(&check, pt)
        : 0;
}

plugin_ui_touch_track_t plugin_ui_control_touch_track(plugin_ui_control_t control) {
    plugin_ui_touch_track_t track;
    plugin_ui_env_t env = control->m_page->m_env;

    TAILQ_FOREACH(track, &env->m_touch_tracks, m_next_for_env) {
        if (track->m_catch_control == control) return track;
    }
    
    return NULL;
}

void plugin_ui_control_adj_by_new_parent(plugin_ui_control_t control, plugin_ui_control_t parent) {
    UI_UNIT_VECTOR_2 sz;
    UI_UNIT_RECT pd;

    plugin_ui_calc_child_unit(
        &control->m_render_sz,
        cpe_ba_get(&control->m_flag, plugin_ui_control_flag_accept_ptls),
        &parent->m_editor_sz,
        &parent->m_editor_sz,
        &control->m_editor_sz);

    //control->m_client_sz = control->m_render_sz;

    plugin_ui_calc_child_unit(
        &control->m_client_pd.lt,
        cpe_ba_get(&control->m_flag, plugin_ui_control_flag_accept_ptls),
        &parent->m_editor_sz,
        &parent->m_editor_sz,
        &control->m_editor_pd.lt);

    plugin_ui_calc_child_unit(
        &control->m_client_pd.rb,
        cpe_ba_get(&control->m_flag, plugin_ui_control_flag_accept_ptls),
        &parent->m_editor_sz,
        &parent->m_editor_sz,
        &control->m_editor_pd.rb);

    plugin_ui_calc_child_unit(
        &sz,
        plugin_ui_control_accept_szls(control),
        &parent->m_editor_sz,
        &parent->m_editor_sz,
        &control->m_editor_sz);

    plugin_ui_calc_child_unit(
        &pd.lt,
        plugin_ui_control_accept_ptls(control),
        &parent->m_editor_sz,
        &parent->m_editor_sz,
        &control->m_editor_pd.lt);

    plugin_ui_calc_child_unit(
        &pd.rb,
        plugin_ui_control_accept_ptls(control),
        &parent->m_editor_sz,
        &parent->m_editor_sz,
        &control->m_editor_pd.rb);

	control->m_render_sz = sz;
	control->m_client_sz = sz;
	control->m_client_pd = pd;
}

uint8_t plugin_ui_control_is_name_eq_no_category(plugin_ui_control_t control, const char * name) {
    return plugin_ui_control_name_is_eq_no_category(control->m_page->m_env, plugin_ui_control_name(control), name);
}

uint8_t plugin_ui_control_name_is_eq_no_category(plugin_ui_env_t env, const char * check_name, const char * name) {
    if (cpe_str_start_with(check_name, name)) {
        size_t name_len = strlen(name);
        if (check_name[name_len] == 0) return 1;

        if (check_name[name_len] == '_'
            && plugin_ui_control_category_find(env, check_name + name_len + 1) != NULL)
        {
            return 1;
        }
    }

    return 0;
}

uint8_t plugin_ui_control_is_attr_eq(plugin_ui_control_t control, const char * attr_name, dr_value_t attr_value) {
    struct dr_value check_value;
    plugin_ui_control_attr_meta_t attr_meta;

    attr_meta = plugin_ui_control_attr_meta_find(control->m_meta, attr_name);
    if (attr_meta == NULL) return 0;
    if (attr_meta->m_getter == NULL) return 0;
    if (attr_meta->m_getter(control, &check_value) != 0) return 0;

    if (check_value.m_type <= CPE_DR_TYPE_COMPOSITE
        && attr_value->m_type <= CPE_DR_TYPE_COMPOSITE)
    {
        if (attr_value->m_meta == check_value.m_meta
            && dr_meta_data_cmp(check_value.m_data, attr_value->m_data, attr_value->m_meta) == 0)
        {
            return 1;
        }
    }
    else if (check_value.m_type > CPE_DR_TYPE_COMPOSITE && attr_value->m_type > CPE_DR_TYPE_COMPOSITE) {
        if (dr_ctype_cmp(check_value.m_data, check_value.m_type, attr_value->m_data, attr_value->m_type) == 0) {
            return 0;
        }
    }

    return 0;
}

uint8_t plugin_ui_control_is_attr_eq_str(plugin_ui_control_t control, const char * attr_name, const char * attr_value) {
    struct dr_value v;

    v.m_type = CPE_DR_TYPE_STRING;
    v.m_meta = NULL;
    v.m_data = (void*)attr_value;
    v.m_size = strlen(attr_value) + 1;

    return plugin_ui_control_is_attr_eq(control, attr_name, &v);
}

uint8_t plugin_ui_control_is_condition_match(plugin_ui_control_t control, const char * condition) {
    return plugin_ui_control_calc_bool_with_dft(condition, control, NULL, 0);
}

ui_vector_2 plugin_ui_control_calc_local_pt_by_policy(plugin_ui_control_t control, uint8_t policy) {
    return plugin_ui_calc_adj_sz_by_pos_policy(plugin_ui_control_real_sz_no_scale(control), &control->m_pivot, policy);
}
