#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_control_meta.h"
#include "RGUIWindow.h"
#include "RGUIToggle.h"
#include "RGUIComboBoxDropList.h"
#include "RGUIComboBox.h"

/*
constructor
*/
RGUIComboBox::RGUIComboBox() {
	mDropListBoxTemplateName = 	"";
	mDropPushBtnTemplateName = 	"";

	mDropListBoxTemplate = NULL;
	mDropPushBtnTemplate = NULL;

	mDropListBox		 = create<RGUIComboBoxDropList>();
	mDropPushBtn		 = create<RGUIToggle>();
	AddChild(mDropListBox);
	AddChild(mDropPushBtn);
}

/*
load & save & clone
*/
void RGUIComboBox::Load(ui_data_control_t control) {
    RGUILabel::Load(control);

    if (type() == ui_control_type_combo_box) {
        UI_CONTROL const & data = *ui_data_control_data(control);
        RGUILabel::Load(data.data.combo_box.text);
        mDropListBoxTemplateName = ui_data_control_msg(control, data.data.combo_box.drop_list_box_template_id);
        mDropPushBtnTemplateName = ui_data_control_msg(control, data.data.combo_box.drop_push_btn_template_id);
    }
}

/*
call back
*/
void RGUIComboBox::OnLoadProperty() {
	RGUILabel::OnLoadProperty();
    
	//得到模板指针 
	mDropPushBtnTemplate = GetTemplate(mDropPushBtnTemplateName.c_str());
	if (mDropPushBtnTemplate)
    {
		mDropPushBtn->SetTemplateLinkCtrl(mDropPushBtnTemplate);
        mDropPushBtn->SetAlignHorz(ui_align_mode_horz_right);
        mDropPushBtn->SetAlignVert(ui_align_mode_vert_center);
        mDropPushBtn->SetParentClip(true);
    }
	mDropListBoxTemplate = GetTemplate(mDropListBoxTemplateName.c_str());
	if (mDropListBoxTemplate)
    {
		mDropListBox->SetTemplateLinkCtrl(mDropListBoxTemplate);
        mDropListBox->SetParentClip(true);
    }
}

/*
event
*/
void RGUIComboBox::on_lost_focus(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    if (ctx == from_control) {
        //RGUIComboBox * c = cast<RGUIComboBox>((plugin_ui_control_t)ctx);

        assert(0);
        //失去焦点时隐藏下拉列表
        //TODO:
        // RGUIControl* other = ((RGUIFocusEventArgs&)args).other;
        // if (!WasDescendant(other))
        //     c->mDropListBox->Hide();
    }
}

void RGUIComboBox::on_hide(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    RGUIComboBox * c = cast<RGUIComboBox>((plugin_ui_control_t)ctx);
	if (from_control == c->mDropListBox->control()) {
		if (c->mDropPushBtn->WasPushed())
			c->mDropPushBtn->SetPushed(false);
	}
}

void RGUIComboBox::ProcessToggleClick(void) {
    //打开下拉列表 
    if (mDropPushBtn->WasPushed()) {
        RVector2 pt = GetRenderRealPTAbs();

        //布局下拉列表在组合框下面 
        if (pt.y  + GetRenderRealH() + mDropListBox->GetRenderRealH() <=
            plugin_ui_env_runtime_sz(ui_env())->y)
            pt.y += GetRenderRealH();
        //布局下拉列表在组合框上面 
        else
            pt.y -= mDropListBox->GetRenderRealH();

        //设值下拉列表位置 
        mDropListBox->SetRenderRealPT(GetRenderRealPTRel(pt));
        //置于顶层，显示，得到焦点 
        mDropListBox->Show();
        mDropListBox->ReqFocus();
    }
    else {
        //隐藏下拉列表 
        mDropListBox->Hide();
    }
}

void RGUIComboBox::on_toggle_click(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    RGUIComboBox * c = cast<RGUIComboBox>((plugin_ui_control_t)ctx);

    if (from_control == c->mDropPushBtn->control()) {
        c->ProcessToggleClick();
    }
}

void RGUIComboBox::on_item_selected(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    RGUIComboBox * c = cast<RGUIComboBox>((plugin_ui_control_t)ctx);
    
	//设值下拉框文字 
	if (from_control == c->mDropListBox->control()) {
		RGUIListBoxAdvItem * item = cast<RGUIListBoxAdvItem>(from_control);
		if (item) {
			// RGUILabel*  label = dynamic_cast<RGUILabel*>(item->GetChildByRTTI(RGUILabel::sRTTI));
			// if (label) c->SetTextA(label->GetTextA());
		}
	}
}

void RGUIComboBox::setup(plugin_ui_control_meta_t meta) {
    RGUILabel::setup(meta);

    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_hide, plugin_ui_event_scope_childs, on_hide);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_lost_focus, plugin_ui_event_scope_childs, on_lost_focus);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_toggle_click, plugin_ui_event_scope_childs, on_toggle_click);    
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_list_selection_changed, plugin_ui_event_scope_childs, on_item_selected);
}
