#ifndef DROW_UI_CONTROL_COMBOBOXDROPDOWNLIST_H
#define DROW_UI_CONTROL_COMBOBOXDROPDOWNLIST_H
#include "RGUIListBoxCol.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUIComboBoxDropList, ui_control_type_comb_box_drop_list);

class RGUIComboBoxDropList : public RGUIListBoxCol {
public:
	/*
	constructor
	*/
	RGUIComboBoxDropList();

	/*
	method
	*/
	uint32_t GetMaxDropItem(void) const { return mMaxDropItem; }
	void SetMaxDropItem(uint32_t maxItem) {
        if (mMaxDropItem != maxItem) { mMaxDropItem  = maxItem; }
    }


	/* 
	load & save
	*/
    virtual void                Load                ( ui_data_control_t control );

protected:
	/*
	event
	*/
	virtual void OnLoadProperty(void);

    static void on_mouse_rise(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void setup(plugin_ui_control_meta_t meta);

	/*
	method
	*/
	virtual void				RenderTail			( ui_runtime_render_t ctx, ui_rect_t rect );

protected:
	/*
	member
	*/
	uint32_t	mMaxDropItem;

protected:
	/*
	hidden destructor
	*/
	virtual ~RGUIComboBoxDropList( void ){};
    friend class RGUIControlRepo; 
};

#endif//__RGUICOMBOBOXDROPLIST_H__
