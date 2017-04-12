#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_stdlib.h"
#include "RGUIWindow.h"
#include "RGUILabelCondition.h"

RGUILabelCondition::RGUILabelCondition() : mIndex(-1) {
}

void RGUILabelCondition::DelRenderText(uint32_t index) {
	mRenderTextVec.erase(mRenderTextVec.begin()+index);
}

void RGUILabelCondition::SetRenderText(uint32_t index, const RenderText& text) {
	assert(index < mRenderTextVec.size());
	mRenderTextVec[index] = text;

    if (mIndex == index) {
        SetTextA(mRenderTextVec[mIndex].text.c_str());
    }
}

void	RGUILabelCondition::AddRenderText		( const RenderText& text )
{
	mRenderTextVec.push_back(text);
}

void	RGUILabelCondition::DelRenderTextAll	( void )
{
	mRenderTextVec.clear();
}


/*
load & save & clone
*/
void RGUILabelCondition::Load(ui_data_control_t control) {
    RGUILabel::Load(control);

    if (type() == ui_control_type_label_condition) {
        UI_CONTROL const & data = *ui_data_control_data(plugin_ui_control_src(this->control()));
        mIndex = data.data.label_condition.index;

        ui_data_control_addition_it addition_it;
        ui_data_control_additions(&addition_it, control);
        while(ui_data_control_addition_t addition = ui_data_control_addition_it_next(&addition_it)) {
            UI_CONTROL_ADDITION const * addition_data = ui_data_control_addition_data(addition);
            if (addition_data->type != ui_control_addition_type_text) continue;

            RenderText text;
            text.text = ui_data_control_msg(control, addition_data->data.text.text_id);
            text.tkey =  addition_data->data.text.text_key;
            //text.textDraw = addition_data->data.text.text_drow;

            mRenderTextVec.push_back(text);
        }
    }
}

/*
virtual
*/
void RGUILabelCondition::Retext(void) {
	for (uint32_t i = 0; i < mRenderTextVec.size(); ++i) {
		if (mRenderTextVec[i].tkey != 0) {
            mRenderTextVec[i].text = visibleMsg(mRenderTextVec[i].tkey);
        }
	}
}

void RGUILabelCondition::SetIndex(uint32_t index) {
	if (mIndex != index) {
		mIndex  = index;

        if (mIndex < mRenderTextVec.size()) {
            SetTextA(mRenderTextVec[mIndex].text.c_str());
            //SetTextBackDraw(mRenderTextVec[mIndex].textDraw);
        }
        else {
            SetTextA("");
        }
	}
}

