#ifndef DROW_UI_CONTROL_COMBOBOX_H
#define DROW_UI_CONTROL_COMBOBOX_H
#include <string>
#include "RGUILabel.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUIComboBox, ui_control_type_combo_box);

class RGUIComboBox : public RGUILabel {
public:
	/*
	constructor
	*/
	RGUIComboBox(void);

	/*
	method
	*/
	RGUIComboBoxDropList* GetDropListBox( void ) { return mDropListBox; }
	RGUIToggle * GetDropPushBtn(void) { return mDropPushBtn; }

	/*
	load & save & clone
	*/
    virtual void                        Load                            ( ui_data_control_t control );

protected:
	/*
	call back
	*/
	virtual void OnLoadProperty();

protected:
	void ProcessToggleClick();

    static void on_toggle_click(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);    
    static void on_item_selected(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
	static void on_hide(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void on_lost_focus(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void setup(plugin_ui_control_meta_t meta);
    
protected:
	/*
	member
	*/
	RGUIComboBoxDropList*				mDropListBox;
	RGUIToggle*						mDropPushBtn;
	ui_data_control_t				mDropListBoxTemplate;
	ui_data_control_t               mDropPushBtnTemplate;
	std::string		mDropListBoxTemplateName;
	std::string		mDropPushBtnTemplateName;

protected:
	/*
	hidden destructor
	*/
	virtual ~RGUIComboBox( void ){};
    friend class RGUIControlRepo; 
};

#endif//__RGUICOMBOBOX_H__
