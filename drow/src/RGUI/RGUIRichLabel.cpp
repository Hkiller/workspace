#include "RGUIRichLabel.h"

/*
constructor
*/
RGUIRichLabel::RGUIRichLabel()
{
}

/*
virtual
*/
void	RGUIRichLabel::SetIndex		( uint32_t index )
{

}

/*
call back
*/
void	RGUIRichLabel::UpdateSelf		( float deltaTime )
{
	RGUILabelCondition::UpdateSelf(deltaTime);
/*
	UString text;
	for (uint32_t i = 0; i < mRenderTextVec.size(); ++i)
		text += mRenderTextVec[i].text;

	if (mText != text)
		SetTextW(text);
 */
}
