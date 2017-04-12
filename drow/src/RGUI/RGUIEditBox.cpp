#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_touch_track.h"
#include "plugin/ui/plugin_ui_control_meta.h"
#include "plugin/layout/plugin_layout_render.h"
#include "plugin/editor/plugin_editor_editing.h"
#include "plugin/editor/plugin_editor_module.h"
#include "gdpp/app/Log.hpp"
#include "RGUIEditBox.h"

RGUIEditBox::RGUIEditBox() {
	mMaxLength = 	1024;
	mPassword = 	false;
	mReadOnly = 	false;
	mNumberOnly = 	false;
    m_is_rending_hit = 0;
    SetTextAlign(plugin_layout_align_left_center);
}

RGUIEditBox::~RGUIEditBox( void ) {
}

/*
load & save & clone
*/
void RGUIEditBox::Load( ui_data_control_t control ) {
    RGUILabel::Load(control);

    if (type() == ui_control_type_edit_box) {
        UI_CONTROL const & data = *ui_data_control_data(control);
        RGUILabel::Load(data.data.edit_box.text);
        Load(data.data.edit_box.editor);
    }
}

void RGUIEditBox::Load( UI_CONTROL_EDITOR const & data ) {
    mMaxLength = data.max_length;
    mPassword = data.is_passwd ? true : false;
    mReadOnly = data.is_read_only ? true : false;
    mNumberOnly = data.is_number_only ? true : false;
    mHintText = plugin_ui_control_msg(control(), data.hint_text_id);
    cvt_font_draw(m_saved_font_draw, data.hint_draw);

    if (GetTextLenA() == 0) {
        SetRenderHint(true);
    }
}

/*
override
*/
void	RGUIEditBox::Update			( float deltaTime )
{
	RGUILabel::Update(deltaTime);

    plugin_layout_render_t render = layout_render(m_text_frame);

    plugin_editor_editing_t editing = plugin_editor_editing_find_first(render);
    if (editing) {
        if (editing && !plugin_editor_editing_is_active(editing)) {
            stopEditing();
        }
    }

	//更新光标 
	// if (HasFocus())
	// 	mCaret->Update(deltaTime);
}

/*
call back
*/
void RGUIEditBox::OnTextChanged() {
	//事件
    plugin_ui_control_dispatch_event(
        control(), control(),
        plugin_ui_event_editorbox_changed, plugin_ui_event_dispatch_to_self_and_parent);
}

void RGUIEditBox::on_lost_focus(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    if (ctx == from_control) {
        RGUIEditBox * c = cast<RGUIEditBox>(from_control);
        c->stopEditing();
    }
}

plugin_layout_font_draw_t RGUIEditBox::GetHintTextDraw(void) const {
    if (m_is_rending_hit) {
        return GetTextBackDraw();
    }
    else {
        return const_cast<plugin_layout_font_draw_t>(&m_saved_font_draw);
    }
}

inline
void RGUIEditBox::SetHintTextDraw(const plugin_layout_font_draw& draw) {
    if (m_is_rending_hit) {
        SetTextBackDraw(draw);
    }
    else {
        m_saved_font_draw = draw;
    }
}

void RGUIEditBox::SetRenderHint(bool flag) {
    if (flag) {
        if (!m_is_rending_hit) {
            plugin_layout_render_t layout = layout_render(m_text_frame);
            plugin_layout_render_set_data(layout, mHintText.c_str());

            plugin_layout_font_draw t = m_saved_font_draw;
            
            plugin_layout_font_draw_t font_draw = GetTextBackDraw();
            if (font_draw) m_saved_font_draw = *font_draw;
            update_layout_draw(m_text_frame, &t);
        
            m_is_rending_hit = 1;
        }
    }
    else {
        if (m_is_rending_hit) {
            plugin_layout_render_t layout = layout_render(m_text_frame);
            plugin_layout_render_set_data(layout, "");

            plugin_layout_font_draw t = m_saved_font_draw;
            
            plugin_layout_font_draw_t font_draw = GetTextBackDraw();
            if (font_draw) m_saved_font_draw = *font_draw;
            update_layout_draw(m_text_frame, &t);
        
            m_is_rending_hit = 0;
        }
    }
}

plugin_editor_editing_t RGUIEditBox::editing(void) {
    return plugin_editor_editing_find_first(layout_render(m_text_frame));
}

plugin_editor_editing_t
RGUIEditBox::startEditing(void) {
    bool save_render_hit = WasRenderHint();
    plugin_layout_render_t render = layout_render(m_text_frame);

    plugin_editor_editing_t editing = plugin_editor_editing_find_first(render);
    if (editing == NULL) {
        if (save_render_hit) SetRenderHint(false);
        
        editing = create_editing(layout_render(m_text_frame));
        if (editing == NULL) {
            if (save_render_hit) SetRenderHint(true);
            return NULL;
        }
        
        plugin_editor_editing_set_number_only(editing, mNumberOnly ? 1 : 0);
        plugin_editor_editing_set_is_passwd(editing, mPassword ? 1 : 0);
    }

    if (!plugin_editor_editing_is_active(editing)) {
        plugin_editor_editing_set_is_active(editing, 1);
    }

    return editing;
}

void RGUIEditBox::stopEditing(void) {
    plugin_layout_render_t render = layout_render(m_text_frame);

    plugin_editor_editing_t editing = plugin_editor_editing_find_first(render);
    if (editing) {
        if (GetTextLenA() == 0) {
            SetRenderHint(true);
        }
        
        plugin_editor_editing_free(editing);
    }
}

void RGUIEditBox::on_mouse_down(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
	RGUILabel::on_mouse_down(ctx, from_control, event);

    if (ctx == from_control) {
        RGUIEditBox * c = cast<RGUIEditBox>(from_control);
        if (!c->mReadOnly) {
            plugin_editor_editing_t editing = c->startEditing();
            if (editing) {
                //设置光标
                plugin_ui_touch_track_t track = plugin_ui_control_touch_track(from_control);
                assert(track);

                ui_vector_2_t adj = plugin_ui_env_screen_adj(plugin_ui_control_env(from_control));
                ui_vector_2_t base_pt = plugin_ui_control_real_pt_abs(from_control);
                ui_vector_2 pt = *plugin_ui_touch_track_down_pt(track);
                pt.x -= base_pt->x;
                pt.y -= base_pt->y;

                pt.x /= adj->x;
                pt.y /= adj->y;

                plugin_editor_editing_selection_update(editing, &pt, &pt);
            }
        }
    }
}

void RGUIEditBox::on_mouse_drag(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    if (ctx == from_control) {
        RGUIEditBox * c = cast<RGUIEditBox>(from_control);
        plugin_editor_editing_t editing = c->editing();
        if (editing) {
            plugin_ui_touch_track_t track = plugin_ui_control_touch_track(from_control);
            assert(track);

            ui_vector_2_t adj = plugin_ui_env_screen_adj(plugin_ui_control_env(from_control));
            ui_vector_2_t base_pt = plugin_ui_control_real_pt_abs(from_control);

            ui_vector_2 down_pt = *plugin_ui_touch_track_down_pt(track);
            down_pt.x -= base_pt->x;
            down_pt.y -= base_pt->y;
            down_pt.x /= adj->x;
            down_pt.y /= adj->y;

            ui_vector_2 cur_pt = *plugin_ui_touch_track_cur_pt(track);
            cur_pt.x -= base_pt->x;
            cur_pt.y -= base_pt->y;
            cur_pt.x /= adj->x;
            cur_pt.y /= adj->y;
            
            plugin_editor_editing_selection_update(editing, &down_pt, &cur_pt);
        }            
	}
}

void RGUIEditBox::setup(plugin_ui_control_meta_t meta) {
    RGUILabel::setup(meta);
    
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_down, plugin_ui_event_scope_self, on_mouse_down);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_move, plugin_ui_event_scope_self, on_mouse_drag);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_lost_focus, plugin_ui_event_scope_self, on_lost_focus);
}
