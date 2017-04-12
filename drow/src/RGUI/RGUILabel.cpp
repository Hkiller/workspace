#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpepp/utils/MemBuffer.hpp"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_ucs4.h"
#include "cpe/dr/dr_data_value.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/layout/plugin_layout_font_info.h"
#include "plugin/layout/plugin_layout_render.h"
#include "plugin/layout/plugin_layout_layout.h"
#include "plugin/layout/plugin_layout_layout_basic.h"
#include "plugin/layout/plugin_layout_layout_rich.h"
#include "plugin/editor/plugin_editor_module.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_aspect.h"
#include "plugin/ui/plugin_ui_control_frame.h"
#include "plugin/ui/plugin_ui_control_attr_meta.h"
#include "gdpp/app/Log.hpp"
#include "RGUILabel.h"

/*
constructor
*/
RGUILabel::RGUILabel(const char * layout)
	: m_text_frame(create_text_frame(layout, plugin_ui_control_frame_usage_normal))
{
}

RGUILabel::~RGUILabel() {
}

const char * RGUILabel::GetTextA(void) const {
    const char * r = plugin_layout_render_data(layout_render(m_text_frame));
    return r ? r : "";
}

size_t RGUILabel::GetTextLenA(void) const {
    return plugin_layout_render_data_len(layout_render(m_text_frame));
}

void RGUILabel::SetTextA(const char * text) {
    plugin_layout_render_set_data(layout_render(m_text_frame), text);
    mTextKey = 0;
}

bool RGUILabel::PastFromClipboard(void) {
    return plugin_editor_module_past_from_clipboard(editor_module(), layout_render(m_text_frame)) == 0 ? true : false;
}

bool RGUILabel::CopyToClipboard(void) {
    return plugin_editor_module_copy_to_clipboard(editor_module(), layout_render(m_text_frame)) == 0 ? true : false;
}

plugin_layout_render_t
RGUILabel::layout_render(void) {
    return layout_render(m_text_frame);
}

ui_rect RGUILabel::GetTextBoundRT(void) {
    plugin_layout_render_t render = layout_render(m_text_frame);
    plugin_layout_render_do_layout(render);

    ui_rect r;
    plugin_layout_render_bound_rt(render, &r);

    ui_vector_2_t adj = plugin_ui_env_screen_adj(ui_env());
    r.rb.x *= adj->x;
    r.rb.y *= adj->y;

    return r;
}

bool RGUILabel::GetTextSL(void) const {
    if (plugin_layout_layout_basic_t basic_layout = layout_basic(m_text_frame)) {
        return plugin_layout_layout_basic_line_break(basic_layout) ? false : true;
    }
    else if (plugin_layout_layout_rich_t rich_layout = layout_rich(m_text_frame)) {
        return plugin_layout_layout_rich_line_break(rich_layout) ? false : true;
    }
    else {
        return false;
    }
}

void RGUILabel::SetTextSL(bool flag) {
    if (plugin_layout_layout_basic_t basic_layout = layout_basic(m_text_frame)) {
        return plugin_layout_layout_basic_set_line_break(basic_layout, flag ? 0 : 1);
    }
}

plugin_layout_align_t RGUILabel::GetTextAlign(void) const {
    if (plugin_layout_layout_basic_t basic_layout = layout_basic(m_text_frame)) {
        return plugin_layout_layout_basic_align(basic_layout);
    }
    else if (plugin_layout_layout_rich_t rich_layout = layout_rich(m_text_frame)) {
        return plugin_layout_layout_rich_align(rich_layout);
    }
    else {
        return plugin_layout_align_left_top;
    }
}

void RGUILabel::SetTextAlign(plugin_layout_align_t text_align) {
    if (plugin_layout_layout_basic_t basic_layout = layout_basic(m_text_frame)) {
        return plugin_layout_layout_basic_set_align(basic_layout, text_align);
    }
    else if (plugin_layout_layout_rich_t rich_layout = layout_rich(m_text_frame)) {
        return plugin_layout_layout_rich_set_align(rich_layout, text_align);
    }
}

int RGUILabel::set_font_family(const char * font) {
    plugin_layout_font_id font_id;
    
    if (plugin_layout_layout_basic_t basic_layout = layout_basic(m_text_frame)) {
        font_id = *plugin_layout_layout_basic_font_id(basic_layout);
    }
    else if (plugin_layout_layout_rich_t rich_layout = layout_rich(m_text_frame)) {
        font_id = *plugin_layout_layout_rich_default_font_id(rich_layout);
    }
    else {
        return -1;
    }

    if (cpe_str_start_with(font, "sys::")) {
        font_id.category = plugin_layout_font_category_font;
        font_id.face = atoi(font + 5) - 1;
    }
    else if (cpe_str_start_with(font, "art::")) {
        font_id.category = plugin_layout_font_category_pic;
        font_id.face = atoi(font + 5) - 1;
    }
    else {
        return -1;
    }
    
    if (plugin_layout_layout_basic_t basic_layout = layout_basic(m_text_frame)) {
        plugin_layout_layout_basic_set_font_id(basic_layout, &font_id);
    }
    else if (plugin_layout_layout_rich_t rich_layout = layout_rich(m_text_frame)) {
        plugin_layout_layout_rich_set_default_font_id(rich_layout, &font_id);
    }

	return 0;
}

void RGUILabel::set_font_size(uint8_t font_size) {
    if (plugin_layout_layout_basic_t basic_layout = layout_basic(m_text_frame)) {
        plugin_layout_font_id font_id = *plugin_layout_layout_basic_font_id(basic_layout);
        font_id.size = font_size;
        plugin_layout_layout_basic_set_font_id(basic_layout, &font_id);
    }
    else if (plugin_layout_layout_rich_t rich_layout = layout_rich(m_text_frame)) {
        plugin_layout_font_id font_id = *plugin_layout_layout_rich_default_font_id(rich_layout);
        font_id.size = font_size;
        plugin_layout_layout_rich_set_default_font_id(rich_layout, &font_id);
    }
}

void RGUILabel::adj_font_size(int8_t font_size) {
    if (plugin_layout_layout_basic_t basic_layout = layout_basic(m_text_frame)) {
        plugin_layout_font_id font_id = *plugin_layout_layout_basic_font_id(basic_layout);
        font_id.size += font_size;
        plugin_layout_layout_basic_set_font_id(basic_layout, &font_id);
    }
    else if (plugin_layout_layout_rich_t rich_layout = layout_rich(m_text_frame)) {
        plugin_layout_font_id font_id = *plugin_layout_layout_rich_default_font_id(rich_layout);
        font_id.size += font_size;
        plugin_layout_layout_rich_set_default_font_id(rich_layout, &font_id);
    }
}

void RGUILabel::SetNormalTextAlpha(float alpha) {
    if (plugin_layout_layout_basic_t basic_layout = layout_basic(m_text_frame)) {
        plugin_layout_font_draw font_draw = *plugin_layout_layout_basic_font_draw(basic_layout);
        font_draw.color.a = alpha;
        font_draw.stroke_color.a = alpha;
        plugin_layout_layout_basic_set_font_draw(basic_layout, &font_draw);
    }
    else if (plugin_layout_layout_rich_t rich_layout = layout_rich(m_text_frame)) {
        plugin_layout_font_draw font_draw = *plugin_layout_layout_rich_default_font_draw(rich_layout);
        font_draw.color.a = alpha;
        font_draw.stroke_color.a = alpha;
        plugin_layout_layout_rich_set_default_font_draw(rich_layout, &font_draw);
    }
}

void RGUILabel::Load( ui_data_control_t control ) {
    RGUIControl::Load(control);

    if (type() == ui_control_type_label) {
        UI_CONTROL const & data = *ui_data_control_data(control);
        Load(data.data.label.text);
    }
}

void RGUILabel::Load(UI_CONTROL_TEXT const & data) {
    mTextKey = data.text_key;
    update_layout_font(m_text_frame, data.font_info, data.back_drow);

    plugin_layout_font_draw font_draw;
    cvt_font_draw(font_draw, data.back_drow);
    update_layout_draw(m_text_frame, &font_draw);
    
    SetTextAlign((plugin_layout_align_t)data.text_align);
    SetTextSL(data.text_sl);

    if (mTextKey) {
        plugin_layout_render_set_data(layout_render(m_text_frame), visibleMsg(mTextKey));
    }
    else {
        plugin_layout_render_set_data(layout_render(m_text_frame), plugin_ui_control_msg(control(), data.text_id));
    }
}

void RGUILabel::Retext(void) {
    if (mTextKey == 0) return;
    plugin_layout_render_set_data(layout_render(m_text_frame), visibleMsg(mTextKey));
    OnTextChanged();
}

void RGUILabel::OnTextChanged() {
}

void RGUILabel::SetLayoutSrc(void) {
    plugin_ui_control_frame_set_sync_size(m_text_frame, 0);
    update_layout_sz(m_text_frame, plugin_ui_control_editor_sz(control()));
}

plugin_layout_font_draw_t RGUILabel::GetTextBackDraw(void) const {
    if (plugin_layout_layout_basic_t basic_layout = layout_basic(m_text_frame)) {
        return plugin_layout_layout_basic_font_draw(basic_layout);
    }
    else if (plugin_layout_layout_rich_t rich_layout = layout_rich(m_text_frame)) {
        return plugin_layout_layout_rich_default_font_draw(rich_layout);
    }
    else {
        return NULL;
    }
}

uint8_t RGUILabel::WasAutoLayout(void) const {
    return  plugin_ui_control_frame_sync_size(m_text_frame);
}

void RGUILabel::SetAutoLayout(uint8_t auto_layout) {
    plugin_ui_control_frame_set_sync_size(m_text_frame, auto_layout);
}

void RGUILabel::update_layout_font(plugin_ui_control_frame_t frame, UI_FONT const & font_info, UI_FONT_DROW const & font_draw) {
    plugin_layout_font_id font_id;
    cvt_font_id(font_id, font_info, font_draw);

    if (plugin_layout_layout_basic_t basic_layout = layout_basic(frame)) {
        plugin_layout_layout_basic_set_font_id(basic_layout, &font_id);
    }
    else if (plugin_layout_layout_rich_t rich_layout = layout_rich(frame)) {
        plugin_layout_layout_rich_set_default_font_id(rich_layout, &font_id);
    }
}

void RGUILabel::update_layout_align(plugin_ui_control_frame_t frame, plugin_layout_align_t align) {
    if (plugin_layout_layout_basic_t basic_layout = layout_basic(frame)) {
        plugin_layout_layout_basic_set_align(basic_layout, align);
    }
    else if (plugin_layout_layout_rich_t rich_layout = layout_rich(frame)) {
        plugin_layout_layout_rich_set_align(rich_layout, align);
    }
}

void RGUILabel::update_layout_draw(plugin_ui_control_frame_t frame, plugin_layout_font_draw_t font_draw) {
    if (plugin_layout_layout_basic_t basic_layout = layout_basic(frame)) {
        plugin_layout_layout_basic_set_font_draw(basic_layout, font_draw);
    }
    else if (plugin_layout_layout_rich_t rich_layout = layout_rich(frame)) {
        plugin_layout_layout_rich_set_default_font_draw(rich_layout, font_draw);
    }
}

void RGUILabel::update_layout_sz(plugin_ui_control_frame_t frame, ui_vector_2_t sz) {
    plugin_layout_render_t render = layout_render(frame);
    plugin_layout_render_set_size(render, sz);
}

plugin_layout_layout_basic_t
RGUILabel::layout_basic(plugin_ui_control_frame_t frame) {
    plugin_layout_render_t render = layout_render(frame);
    if (render == NULL) return NULL;

    plugin_layout_layout_t layout = plugin_layout_render_layout(render);
    if (layout == NULL) return NULL;

    if (strcmp(plugin_layout_layout_meta_name(layout), "basic") != 0) return NULL;
    
    return (plugin_layout_layout_basic_t)plugin_layout_layout_data(layout);
}

plugin_layout_layout_rich_t
RGUILabel::layout_rich(plugin_ui_control_frame_t frame) {
    plugin_layout_render_t render = layout_render(frame);
    if (render == NULL) return NULL;

    plugin_layout_layout_t layout = plugin_layout_render_layout(render);
    if (layout == NULL) return NULL;

    if (strcmp(plugin_layout_layout_meta_name(layout), "rich") != 0) return NULL;
    
    return (plugin_layout_layout_rich_t)plugin_layout_layout_data(layout);
}

plugin_layout_render_t
RGUILabel::layout_render(plugin_ui_control_frame_t frame) {
    ui_runtime_render_obj_t render_obj
        = ui_runtime_render_obj_ref_obj(plugin_ui_control_frame_render_obj_ref(frame));
    
    return (plugin_layout_render_t)ui_runtime_render_obj_data(render_obj);
}

plugin_ui_control_frame_t
RGUILabel::create_text_frame(const char * layout, plugin_ui_control_frame_usage_t usage) {
    ui_runtime_render_obj_t render_obj = ui_runtime_render_obj_create_by_type(runtime(), NULL, "layout");
    if (render_obj == NULL) return NULL;

    plugin_layout_render_t render = (plugin_layout_render_t)ui_runtime_render_obj_data(render_obj);
    plugin_layout_render_set_layout(render, layout);

    ui_runtime_render_obj_ref_t render_obj_ref = ui_runtime_render_obj_ref_create_by_obj(render_obj);
    if (render_obj_ref == NULL) return NULL;
    
    plugin_ui_control_frame_t frame =
        plugin_ui_control_frame_create(
            control(), plugin_ui_control_frame_layer_text, usage, render_obj_ref);
    if (frame == NULL) return NULL;

    plugin_ui_control_frame_set_sync_size(frame, true);

    plugin_ui_control_frame_set_base_pos(frame, ui_pos_policy_top_left);
    plugin_ui_control_frame_set_sync_size(frame, 1);
    plugin_ui_aspect_control_frame_add(lock_aspect(), frame, 1);
    
    return frame;
}

void RGUILabel::cvt_font_draw(plugin_layout_font_draw & o, UI_FONT_DROW const & i) {
    o.color.r = i.normal_color.r;
    o.color.g = i.normal_color.g;
    o.color.b = i.normal_color.b;
    o.color.a = i.normal_color.a;
    o.stroke_color.r = i.stroke_color.r;
    o.stroke_color.g = i.stroke_color.g;
    o.stroke_color.b = i.stroke_color.b;
    o.stroke_color.a = i.stroke_color.a;
    o.grap_horz = i.horz_grap;
	o.grap_vert = i.vert_grap;

    o.flag = 0;

    if (i.drow_flag & 0x01) {
        o.flag |= plugin_layout_font_draw_flag_shadow;
    }

    if (i.drow_flag & 0x10) {
        o.flag |= plugin_layout_font_draw_flag_undline;
    }
}

void RGUILabel::cvt_font_id(plugin_layout_font_id & o, UI_FONT const & i, UI_FONT_DROW const & font_draw) {
    o.category = i.artfile ? plugin_layout_font_category_pic : plugin_layout_font_category_font;
    o.face = i.face;
	o.size = i.size;

    if (font_draw.drow_flag & 0x02) {
        o.stroke_width = font_draw.stroke_width;
    }
    else {
        o.stroke_width = 0;
    }
}

int RGUILabel::text_setter(plugin_ui_control_t control, dr_value_t value) {
    Cpe::Utils::MemBuffer buffer(NULL);
    const char * str_value = dr_value_to_string(buffer, value, NULL);
    
    cast<RGUILabel>(control)->SetTextA(str_value);
    
    return 0;
}

int RGUILabel::text_id_setter(plugin_ui_control_t control, dr_value_t value) {
    uint32_t text_id = dr_value_read_with_dft_uint32(value, 0u);
    cast<RGUILabel>(control)->SetTextKey(text_id);
    return 0;
}

int RGUILabel::text_font_size_setter(plugin_ui_control_t control, dr_value_t value) {
    Cpe::Utils::MemBuffer buffer(NULL);
    const char * str_value = dr_value_to_string(buffer, value, NULL);
    if (str_value[0] == '+') {
        cast<RGUILabel>(control)->adj_font_size(atoi(str_value + 1));
    }
    else if (str_value[0] == '-') {
        cast<RGUILabel>(control)->adj_font_size(- atoi(str_value + 1));
    }
    else {
        cast<RGUILabel>(control)->set_font_size(atoi(str_value));
    }
    
    return 0;
}

int RGUILabel::text_font_family_setter(plugin_ui_control_t control, dr_value_t value) {
    Cpe::Utils::MemBuffer buffer(NULL);
    const char * str_value = dr_value_to_string(buffer, value, NULL);
    return cast<RGUILabel>(control)->set_font_family(str_value);
}

int RGUILabel::text_alpha_setter(plugin_ui_control_t control, dr_value_t value) {
    float alpha;
    if (dr_value_try_read_float(&alpha, value, NULL) != 0) return -1;
    cast<RGUILabel>(control)->SetNormalTextAlpha(alpha);
    return 0;
}

void RGUILabel::setup(plugin_ui_control_meta_t meta) {
    RGUIControl::setup(meta);

    plugin_ui_control_attr_meta_create(meta, "text", text_setter, NULL);
    plugin_ui_control_attr_meta_create(meta, "text-id", text_id_setter, NULL);
    plugin_ui_control_attr_meta_create(meta, "text-font-size", text_font_size_setter, NULL);
    plugin_ui_control_attr_meta_create(meta, "text-font-family", text_font_family_setter, NULL);
    plugin_ui_control_attr_meta_create(meta, "text-alpha", text_alpha_setter, NULL);
}
