#include "plugin/ui/plugin_ui_control_meta.h"
#include "RGUIWindow.h"
#include "RGUIMultiEditBox.h"

/*
constructor
*/
RGUIMultiEditBox::RGUIMultiEditBox()
{
	mLineHeight =  20;
	mUnitHeight = 	RGUIUnit::Zero;
}

/*
load & save & clone
*/
void RGUIMultiEditBox::Load( ui_data_control_t control ) {
    RGUIEditBox::Load(control);

    if (type() == ui_control_type_multi_edit_box) {
        UI_CONTROL const & data = *ui_data_control_data(control);
        RGUILabel::Load(data.data.multi_edit_box.text);
        RGUIEditBox::Load(data.data.multi_edit_box.editor);

        // if (!mTemplateLink) {
            mLineHeight = data.data.multi_edit_box.line_height;
            mUnitHeight = data.data.multi_edit_box.unit_height;
            // RXmlHelper::ImportValue(node, "LineHeight",	mLineHeight);
            // RXmlHelper::ImportValue(node, "UnitHeight",	mUnitHeight);
        // }
    }
}

/*
virtual
*/
void RGUIMultiEditBox::PerformLayout(ui_vector_2_t client_sz) {
    client_sz->x = client_sz->y = 0.0f;
}

/*
call back
*/
void RGUIMultiEditBox::OnLoadProperty() {
	RGUIEditBox::OnLoadProperty();
    
	OnTextChanged();
}

void RGUIMultiEditBox::OnTextChanged()
{
	//确保最后是换行
    /*
	if (mText.length() == 0 || mText[mText.length()-1] != L'\n')
		mText.append(1, L'\n');

	//回调 
	RGUIEditBox::OnTextChanged();
     */
}

uint32_t	RGUIMultiEditBox::GetLineIndexByIndex		( uint32_t index ) const
{
	uint32_t lineCount = mLineInfoVec.size();

	if (lineCount == 0)
		return 0;

	// if (index >= mText.length())
	// 	return lineCount-1;

	uint32_t textIndex = 0;
	uint32_t lineIndex = 0;
	for (;lineIndex < lineCount; ++lineIndex)
	{
		textIndex  += mLineInfoVec[lineIndex].count;
		if (index  <  textIndex)
			break;
	}

	return lineIndex;
}

void RGUIMultiEditBox::on_layout(plugin_ui_control_t control, ui_vector_2_t client_sz) {
    RGUIMultiEditBox * tab = (RGUIMultiEditBox*)plugin_ui_control_product(control);
    tab->PerformLayout(client_sz);
}

void RGUIMultiEditBox::setup(plugin_ui_control_meta_t meta) {
    plugin_ui_control_meta_set_layout(meta, on_layout);

    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_down, plugin_ui_event_scope_self, on_mouse_down);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_move, plugin_ui_event_scope_self, on_mouse_drag);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_lost_focus, plugin_ui_event_scope_self, on_lost_focus);
}
    
