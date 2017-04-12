#ifndef DROW_UI_CONTROL_RADIOBOX_H
#define DROW_UI_CONTROL_RADIOBOX_H
#include "RGUICheckBox.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUIRadioBox, ui_control_type_radio_box);

class RGUIRadioBox : public RGUICheckBox {
public:
	/*
	constructor
	*/
	RGUIRadioBox();

	/*
	method
	*/
	void						SetGroup			( uint16_t group );
	uint16_t						GetGroup			( void ) const;

	/*
	override
	*/
	virtual void				SetChecked			( bool flag, bool fireEvent = true );

	/*
	load & save & clone
	*/
    virtual void                Load                ( ui_data_control_t control );

protected:
	/*
	call back
	*/
    static void on_mouse_click(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void setup(plugin_ui_control_meta_t meta);

protected:
	/*
	member
	*/
	uint16_t	mGroup;

protected:
	/*
	hidden destructor
	*/
	virtual ~RGUIRadioBox( void ){};
    friend class RGUIControlRepo; 
};

#include "RGUIRadioBox.inl"

#endif
