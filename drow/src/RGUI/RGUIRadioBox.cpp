#include "plugin/ui/plugin_ui_control_meta.h"
#include "RGUIWindow.h"
#include "RGUIRadioBox.h"

/*
constructor
*/
RGUIRadioBox::RGUIRadioBox()
{
	mGroup = 	0;
}

/*
method
*/
void	RGUIRadioBox::SetChecked			( bool flag, bool fireEvent ) 
{
	if (mChecked != flag)
	{
		mChecked  = flag;
	}

    RGUIControl * parent = GetParent();
	if (parent && flag)
	{
        plugin_ui_control_it child_it;
        plugin_ui_control_childs(parent->control(), &child_it);

        for(plugin_ui_control_t child = plugin_ui_control_it_next(&child_it);
            child;
            child = plugin_ui_control_it_next(&child_it))
        {
            RGUIControl * c = (RGUIControl*)plugin_ui_control_product(child);
			RGUIRadioBox* radioBox = dynamic_cast<RGUIRadioBox*>(c);
			if (radioBox && 
				radioBox != this && 
				radioBox->mGroup == mGroup)
			{
				radioBox->SetChecked(false);
			}
		}

		if (fireEvent)
		{
            plugin_ui_control_dispatch_event(
                control(), control(),
                plugin_ui_event_radiobox_changed, plugin_ui_event_dispatch_to_self_and_parent);
		}
	}
}

void RGUIRadioBox::Load( ui_data_control_t control ) {
    RGUICheckBox::Load( control );

    if (type() == ui_control_type_radio_box) {
        UI_CONTROL const & data = *ui_data_control_data(control);
        RGUILabel::Load(data.data.radio_box.text);
        RGUIButton::Load(data.data.radio_box.down);
        RGUICheckBox::Load(data.data.radio_box.check);
        mGroup = data.data.radio_box.group.group;
    }
}

/*
call back
*/
void RGUIRadioBox::on_mouse_click(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
	RGUILabel::on_mouse_click(ctx, from_control, event);

    if (ctx == from_control) {
        RGUIRadioBox * radio_box = cast<RGUIRadioBox>(from_control);
        if (!radio_box->mChecked) radio_box->SetChecked(true);
    }
}

void RGUIRadioBox::setup(plugin_ui_control_meta_t meta) {
    RGUICheckBox::setup(meta);

    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_click, plugin_ui_event_scope_self, on_mouse_click);
}
