#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_utils_i.h"

void plugin_ui_control_basic_layout(plugin_ui_control_t control, ui_vector_2_t client_sz) {
    ui_rect text_rt;
    plugin_ui_control_t child;
    float text_w, text_h;

    if (cpe_ba_get(&control->m_flag, plugin_ui_control_flag_size_zero)) return;

    assert(cpe_ba_get(&control->m_cache_flag, plugin_ui_control_cache_flag_size) == cpe_ba_false);
                      
    text_rt.lt = control->m_client_real_pd.lt;
    ui_vector_2_sub(&text_rt.rb, &control->m_render_sz_ns, &control->m_client_real_pd.rb);
    text_w = text_rt.rb.x - text_rt.lt.x;
    text_h = text_rt.rb.y - text_rt.lt.y;
    if (text_w < 1.0f || text_h < 1.0f) return;

    TAILQ_FOREACH(child, &control->m_childs, m_next_for_parent) {
        ui_vector_2 base;

        assert(cpe_ba_get(&child->m_cache_flag, plugin_ui_control_cache_flag_size) == cpe_ba_false);

        if (cpe_ba_get(&child->m_flag, plugin_ui_control_flag_size_zero)) continue;

        plugin_ui_calc_base(
            &base,
            cpe_ba_get(&child->m_flag, plugin_ui_control_flag_accept_ptls),
            &control->m_render_sz_ns, &control->m_editor_sz);

		switch(child->m_align_horz) {
        case ui_align_mode_horz_left:
            child->m_render_pt_to_p.x = text_rt.lt.x;
            plugin_ui_real_to_unit(&child->m_render_pt.x, child->m_render_pt_to_p.x, base.x);
			break;
        case ui_align_mode_horz_right:
			child->m_render_pt_to_p.x = text_rt.lt.x + (text_w - child->m_render_sz_ns.x);
            plugin_ui_real_to_unit(&child->m_render_pt.x, child->m_render_pt_to_p.x, base.x);
			break;
        case ui_align_mode_horz_center:
			child->m_render_pt_to_p.x = text_rt.lt.x + (text_w - child->m_render_sz_ns.x) / 2.0f;
            plugin_ui_real_to_unit(&child->m_render_pt.x, child->m_render_pt_to_p.x, base.x);
			break;
        case ui_align_mode_horz_none:
        default:
            child->m_render_pt_to_p.x = plugin_ui_unit_to_real(&child->m_render_pt.x, base.x);
            break;
		}

		switch (child->m_align_vert) {
        case ui_align_mode_vert_top:
			child->m_render_pt_to_p.y = text_rt.lt.y;
            plugin_ui_real_to_unit(&child->m_render_pt.y, child->m_render_pt_to_p.y, base.y);
            break;
        case ui_align_mode_vert_center:
			child->m_render_pt_to_p.y = text_rt.lt.y + (text_h - child->m_render_sz_ns.y) / 2.0f;
            plugin_ui_real_to_unit(&child->m_render_pt.y, child->m_render_pt_to_p.y, base.y);
            break;
        case ui_align_mode_vert_bottom:
			child->m_render_pt_to_p.y = text_rt.lt.y + (text_h - child->m_render_sz_ns.y);
            plugin_ui_real_to_unit(&child->m_render_pt.y, child->m_render_pt_to_p.y, base.y);
            break;
        case ui_align_mode_vert_none:
        default:
            child->m_render_pt_to_p.y = plugin_ui_unit_to_real(&child->m_render_pt.y, base.y);
            break;
		}

        /* CPE_ERROR( */
        /*         control->m_page->m_env->m_module->m_em, */
        /*         "        child %s: parent.editor-size: (%f,%f), parent-runtime-size=(%f,%f), size=(%f,%f), pt=(%f,%f)", */
        /*         plugin_ui_control_name(child), */
        /*         control->m_editor_sz.x, control->m_editor_sz.y, */
        /*         control->m_render_sz_ns.x, control->m_render_sz_ns.y, */
        /*         child->m_render_sz_ns.x, child->m_render_sz_ns.y, */
        /*         child->m_render_pt_to_p.x, child->m_render_pt_to_p.y); */
    }
}
