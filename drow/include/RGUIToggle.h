#ifndef DROW_UI_CONTROL_TOGGLE_H
#define DROW_UI_CONTROL_TOGGLE_H
#include "RGUIButton.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUIToggle, ui_control_type_toggle);

class RGUIToggle : public RGUIButton {
public:
	RGUIToggle();

	uint32_t GetGroup(void) const { return mGroup; }
	void SetGroup(uint32_t group) {
        if (mGroup != group) {
            mGroup  = group;
            SetPushed(mPushed);
        }
    }

	uint8_t WasPushed(void) const { return mPushed; }
	void SetPushed(uint8_t flag, uint8_t fire_event = 1);

	uint8_t WasSelectTop(void) const { return mSelectTop; }
	void SetSelectTop(uint8_t flag = 0) { mSelectTop = flag; }

    virtual void Load(ui_data_control_t control);
    
protected:
    static void on_mouse_click(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static int pushed_setter(plugin_ui_control_t control, dr_value_t value);
    static int pushed_getter(plugin_ui_control_t control, dr_value_t value);
    static void setup(plugin_ui_control_meta_t meta);
    
protected:
    uint8_t mProcessing;
	uint8_t mPushed;
	uint32_t mGroup;
	uint8_t mSelectTop;

    ~RGUIToggle();
    
    friend class RGUIControlRepo; 
};

#endif
