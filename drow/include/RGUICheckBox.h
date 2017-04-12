#ifndef DROW_UI_CONTROL_CHECKBOX_H
#define DROW_UI_CONTROL_CHECKBOX_H
#include "RGUIButton.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUICheckBox, ui_control_type_check_box);

class RGUICheckBox : public RGUIButton {
public:
	RGUICheckBox();

	void AddCheckFrame(const char * res);

	virtual void SetChecked(bool flag);
	bool WasChecked(void) const { return mChecked; }

    virtual void Load( ui_data_control_t control );
            void Load( UI_CONTROL_CHECK const & data );

protected:
    static void on_mouse_click(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void setup(plugin_ui_control_meta_t meta);

protected:
	/*
	member
	*/
	bool			mChecked;

protected:
	/*
	hidden destructor
	*/
	virtual ~RGUICheckBox( void ){};
    friend class RGUIControlRepo; 
};

#endif
