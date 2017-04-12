#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "ui_data_layout_i.h"
#include "ui_data_src_src_i.h"
#include "ui_data_src_res_i.h"
#include "ui_data_src_i.h"

static int ui_data_layout_add_use_by_template(ui_data_src_t user, ui_data_control_t control, uint32_t template_id) {
    if (template_id == 0) return 0;
    return ui_data_src_src_create_by_path(user, ui_data_control_msg(control, template_id), ui_data_src_type_layout) ? 0 : -1;
}

int ui_data_layout_add_use_by_ref(ui_data_src_t user, ui_data_control_t control, UI_CONTROL_OBJECT_URL const * ref) {
    UI_OBJECT_URL url;

    if (ref->type == UI_OBJECT_TYPE_NONE) return 0;
    
    if (ui_data_control_ref_to_object_ref(&url, ref) != 0) return -1;
    return ui_data_src_src_create_by_url(user, &url);
}

int ui_data_layout_add_use_by_font(ui_data_src_t user, ui_data_control_t control, UI_FONT const * font_info) {
    int rv = 0;
    char buf[256];
    
    if (font_info->artfile) {
        snprintf(buf, sizeof(buf), "ArtFont/artFont_%d", font_info->face + 1);
        if (ui_data_src_src_create_by_path(user, buf, ui_data_src_type_module) == NULL) rv = -1;
    }
    else {
        if (user->m_mgr->m_cache_mgr) {
            snprintf(buf, sizeof(buf), "sysfont/sysFont_%d.ttf", font_info->face + 1);
            return ui_data_src_res_create_by_path(user, buf) ? 0 : -1;
        }
    }
    
    return rv;
}

int ui_data_layout_add_use_by_data_text(ui_data_src_t user, ui_data_control_t control, UI_CONTROL_TEXT const * text) {
    int rv = 0;

    if (ui_data_layout_add_use_by_font(user, control, &text->font_info) != 0) rv = -1;

    return rv;
}

int ui_data_layout_add_use_by_data_scroll(ui_data_src_t user, ui_data_control_t control, UI_CONTROL_SCROLL const * scroll) {
    int rv = 0;
    
    if (ui_data_layout_add_use_by_ref(user, control, &scroll->v_scroll_bar.res) != 0) rv = -1;
    if (ui_data_layout_add_use_by_ref(user, control, &scroll->v_scroll_mid.res) != 0) rv = -1;
    if (ui_data_layout_add_use_by_ref(user, control, &scroll->h_scroll_bar.res) != 0) rv = -1;
    if (ui_data_layout_add_use_by_ref(user, control, &scroll->h_scroll_mid.res) != 0) rv = -1;
    if (ui_data_layout_add_use_by_template(user, control, scroll->left_btn_template_id) != 0) rv = -1;
    if (ui_data_layout_add_use_by_template(user, control, scroll->right_btn_template_id) != 0) rv = -1;
    if (ui_data_layout_add_use_by_template(user, control, scroll->up_btn_template_id) != 0) rv = -1;
    if (ui_data_layout_add_use_by_template(user, control, scroll->down_btn_template_id) != 0) rv = -1;

    return rv;
}

int ui_data_layout_add_use_by_data_box(ui_data_src_t user, ui_data_control_t control, UI_CONTROL_BOX const * box) {
    int rv = 0;
    
    if (ui_data_layout_add_use_by_ref(user, control, &box->light_frame.res) != 0) rv = -1;
    if (ui_data_layout_add_use_by_template(user, control, box->item_template_id) != 0) rv = -1;
    if (ui_data_layout_add_use_by_template(user, control, box->head_template_id) != 0) rv = -1;
    if (ui_data_layout_add_use_by_template(user, control, box->tail_template_id) != 0) rv = -1;

    return rv;
}

int ui_data_layout_add_use_by_data_editor(ui_data_src_t user, ui_data_control_t control, UI_CONTROL_EDITOR const * editor) {
    int rv = 0;
    
    //if (ui_data_layout_add_use_by_ref(user, control, &box->light_frame.res) != 0) rv = -1;

    return rv;
}

static int ui_data_control_update_refs(ui_data_src_t user, ui_data_control_t control) {
    ui_data_control_t child;
    ui_data_control_addition_t addition;
    int rv = 0;

    if (control->m_data.basic.is_link) {
        if (ui_data_layout_add_use_by_template(user, control, control->m_data.basic.link_control_id) != 0) rv = -1;
    }
    else {
        if (user->m_mgr->m_cache_mgr) {
            if (control->m_data.basic.show_sfx_file_id && ui_data_src_res_create_by_path(user, ui_data_control_msg(control, control->m_data.basic.show_sfx_file_id)) == NULL) rv = -1;
            if (control->m_data.basic.hide_sfx_file_id && ui_data_src_res_create_by_path(user, ui_data_control_msg(control, control->m_data.basic.hide_sfx_file_id)) == NULL) rv = -1;
            if (control->m_data.basic.down_sfx_file_id && ui_data_src_res_create_by_path(user, ui_data_control_msg(control, control->m_data.basic.down_sfx_file_id)) == NULL) rv = -1;
            if (control->m_data.basic.rise_sfx_file_id && ui_data_src_res_create_by_path(user, ui_data_control_msg(control, control->m_data.basic.rise_sfx_file_id)) == NULL) rv = -1;
            if (control->m_data.basic.push_sfx_file_id && ui_data_src_res_create_by_path(user, ui_data_control_msg(control, control->m_data.basic.push_sfx_file_id)) == NULL) rv = -1;
        }
        
        switch(control->m_data.type) {
        case ui_control_type_label:
            if (ui_data_layout_add_use_by_data_text(user, control, &control->m_data.data.label.text) != 0) rv = -1;
            break;
        case ui_control_type_button:
            if (ui_data_layout_add_use_by_data_text(user, control, &control->m_data.data.button.text) != 0) rv = -1;
            break;
        case ui_control_type_toggle:
            if (ui_data_layout_add_use_by_data_text(user, control, &control->m_data.data.toggle.text) != 0) rv = -1;
            break;
        case ui_control_type_progress:
            if (ui_data_layout_add_use_by_data_text(user, control, &control->m_data.data.progress.text) != 0) rv = -1;
            break;
        case ui_control_type_check_box:
            if (ui_data_layout_add_use_by_data_text(user, control, &control->m_data.data.check_box.text) != 0) rv = -1;
            break;
        case ui_control_type_radio_box:
            if (ui_data_layout_add_use_by_data_text(user, control, &control->m_data.data.radio_box.text) != 0) rv = -1;
            break;
        case ui_control_type_combo_box:
            if (ui_data_layout_add_use_by_data_text(user, control, &control->m_data.data.combo_box.text) != 0
                || ui_data_layout_add_use_by_template(user, control, control->m_data.data.combo_box.drop_list_box_template_id) != 0
                || ui_data_layout_add_use_by_template(user, control, control->m_data.data.combo_box.drop_push_btn_template_id) != 0
                ) rv = -1;
            break;
        case ui_control_type_edit_box:
            if (ui_data_layout_add_use_by_data_text(user, control, &control->m_data.data.edit_box.text) != 0) rv = -1;
            if (ui_data_layout_add_use_by_data_editor(user, control, &control->m_data.data.edit_box.editor) != 0) rv = -1;
            break;
        case ui_control_type_multi_edit_box:
            if (ui_data_layout_add_use_by_data_text(user, control, &control->m_data.data.multi_edit_box.text) != 0) rv = -1;
            if (ui_data_layout_add_use_by_data_editor(user, control, &control->m_data.data.multi_edit_box.editor) != 0) rv = -1;
            break;
        case ui_control_type_label_condition:
            if (ui_data_layout_add_use_by_data_text(user, control, &control->m_data.data.label_condition.text) != 0) rv = -1;
            break;
        case ui_control_type_rich_label:
            if (ui_data_layout_add_use_by_data_text(user, control, &control->m_data.data.rich_label.text) != 0) rv = -1;
            break;
        case ui_control_type_rich_text:
            if (ui_data_layout_add_use_by_data_text(user, control, &control->m_data.data.rich_text.text) != 0) rv = -1;
            break;
        case ui_control_type_list_box_col:
            if (ui_data_layout_add_use_by_data_scroll(user, control, &control->m_data.data.list_box_col.scroll) != 0) rv = -1;
            if (ui_data_layout_add_use_by_data_box(user, control, &control->m_data.data.list_box_col.box) != 0) rv = -1;
            break;
        case ui_control_type_list_box_row:
            if (ui_data_layout_add_use_by_data_scroll(user, control, &control->m_data.data.list_box_row.scroll) != 0) rv = -1;
            if (ui_data_layout_add_use_by_data_box(user, control, &control->m_data.data.list_box_row.box) != 0) rv = -1;
            break;
        case ui_control_type_scroll_panel:
            if (ui_data_layout_add_use_by_data_scroll(user, control, &control->m_data.data.scroll_panel.scroll) != 0) rv = -1;
            break;
        case ui_control_type_comb_box_drop_list:
            if (ui_data_layout_add_use_by_data_scroll(user, control, &control->m_data.data.comb_box_drop_list.scroll) != 0) rv = -1;
            if (ui_data_layout_add_use_by_data_box(user, control, &control->m_data.data.comb_box_drop_list.box) != 0) rv = -1;
            break;
        case ui_control_type_swiper:
            if (ui_data_layout_add_use_by_ref(user, control, &control->m_data.data.swiper.curr_page_frame.res) != 0) rv = -1;
            if (ui_data_layout_add_use_by_ref(user, control, &control->m_data.data.swiper.gray_page_frame.res) != 0) rv = -1;
            break;
        case ui_control_type_slider:
            if (ui_data_layout_add_use_by_ref(user, control, &control->m_data.data.slider.status_frame.res) != 0
                || ui_data_layout_add_use_by_template(user, control, control->m_data.data.slider.mid_btn_template_id) != 0
                ) rv = -1;
            break;
        case ui_control_type_switcher:
            if (ui_data_layout_add_use_by_ref(user, control, &control->m_data.data.switcher.turnon_frame.res) != 0
                || ui_data_layout_add_use_by_template(user, control, control->m_data.data.switcher.mid_btn_template_id) != 0
                ) rv = -1;
            break;
        case ui_control_type_tab:
            if (ui_data_layout_add_use_by_template(user, control, control->m_data.data.tab.toggle_template_id) != 0) rv = -1;
            break;
        case ui_control_type_tab_page:
            if (ui_data_layout_add_use_by_template(user, control, control->m_data.data.tab_page.toggle_template_id) != 0) rv = -1;
            break;
        default:
            break;
        }
    }

    TAILQ_FOREACH(addition, &control->m_additions, m_next_for_control) {
        switch(addition->m_data.type) {
        case ui_control_addition_type_animation:
            if (ui_data_layout_add_use_by_ref(user, control, &addition->m_data.data.animation.res) != 0) rv = -1;
            break;
        case ui_control_addition_type_res_ref:
            if (ui_data_layout_add_use_by_ref(user, control, &addition->m_data.data.res_ref.frame.res) != 0) rv = -1;
            break;
        default:
            break;
        }
    }
    
    TAILQ_FOREACH(child, &control->m_childs, m_next_for_parent) {
        if (ui_data_control_update_refs(user, child) != 0) rv = -1;
    }

    return rv;
}

int ui_data_layout_update_using(ui_data_src_t src) {
    ui_data_layout_t layout = ui_data_src_product(src);
    return ui_data_control_update_refs(src, layout->m_root);
}
