#ifndef DROW_UI_CONTROL_LISTBOXROW_H
#define DROW_UI_CONTROL_LISTBOXROW_H
#include "RGUIListBoxBase.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUIListBoxRow, ui_control_type_list_box_row);

class RGUIListBoxRow : public RGUIListBoxBase {
public:
	RGUIListBoxRow();

	uint32_t GetRowCount(void) const;
	void SetRowCount(uint32_t count);

	virtual uint32_t GetItemIndex(int32_t point) const;
	virtual int32_t GetItemPixel(void) const;
	virtual uint32_t GetViewStart(void) const;
	virtual uint32_t GetViewFinal(void) const;

protected:
    virtual void Load(ui_data_control_t control);
	virtual void PerformLayout(ui_vector_2_t client_sz);

	virtual void UpdateSelf(float deltaTime);
	virtual void UpdateScrollRange(ui_vector_2_t client_sz);
    
    static void on_mouse_rise(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);    
    static void on_hscroll_changed(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void setup(plugin_ui_control_meta_t meta);


protected:
	uint32_t	mRowCount;

	virtual ~RGUIListBoxRow(void);
    friend class RGUIControlRepo; 
};

#endif
