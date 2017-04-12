#include "cpe/utils/math_ex.h"
#include "plugin/ui/plugin_ui_control_meta.h"
#include "plugin/ui/plugin_ui_control_frame.h"
#include "RGUIWindow.h"
#include "RGUICheckBox.h"

/*
constructor
*/
RGUICheckBox::RGUICheckBox() {
	mChecked = false;
}

/*
load & save & clone
*/
void RGUICheckBox::Load( ui_data_control_t control ) {
    RGUIButton::Load(control);

    if (type() == ui_control_type_check_box) {
        UI_CONTROL const & data = *ui_data_control_data(control);
        RGUILabel::Load(data.data.check_box.text);
        RGUIButton::Load(data.data.check_box.down);
        Load(data.data.check_box.check);
    }
}

void RGUICheckBox::Load( UI_CONTROL_CHECK const & data ) {
    mChecked = data.checked;

    plugin_ui_control_frame_clear_in_layer(control(), plugin_ui_control_frame_layer_back, lock_aspect());
    if (data.check_frame.res.type != UI_OBJECT_TYPE_NONE) {
        plugin_ui_control_frame_create_by_def(
            control(), plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_down, &data.check_frame);
    }
}

void RGUICheckBox::SetChecked(bool flag) {
	if (mChecked != flag) {
		mChecked  = flag;

        plugin_ui_control_dispatch_event(
            control(), control(),
            plugin_ui_event_checkbox_changed, plugin_ui_event_dispatch_to_self_and_parent);
	}	 
}

void RGUICheckBox::on_mouse_click(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    RGUIButton::on_mouse_click(ctx, from_control, event);

    if (ctx == from_control) {
        RGUICheckBox * check_box = cast<RGUICheckBox>(from_control);
        check_box->SetChecked(!check_box->mChecked);
    }
}

void RGUICheckBox::setup(plugin_ui_control_meta_t meta) {
    RGUIButton::setup(meta);

    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_click, plugin_ui_event_scope_self, on_mouse_click);
    
}
