#ifndef DROW_UI_CONTROL_RICHLABEL_H
#define DROW_UI_CONTROL_RICHLABEL_H
#include "RGUILabelCondition.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUIRichLabel, ui_control_type_rich_label);

class RGUIRichLabel : public RGUILabelCondition {
public:
	/*
	constructor
	*/
	RGUIRichLabel();

	/*
	virtual
	*/
	virtual void			SetIndex			( uint32_t index );

protected:
	/*
	call back
	*/
	virtual void			UpdateSelf			( float deltaTime );

protected:
	/*
	hidden destructor
	*/
	virtual ~RGUIRichLabel( void ){};
    friend class RGUIControlRepo; 
};

#endif
