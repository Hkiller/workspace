#ifndef DROW_UI_CONTROL_SWITCHER_H
#define DROW_UI_CONTROL_SWITCHER_H
#include <string>
#include "RGUIControl.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUISwitch, ui_control_type_switcher);

class RGUISwitch : public RGUIControl {
public:
	RGUISwitch();

	RGUIButton * GetMidBtn(void) { return mMidBtn; }
	bool WasTurnon(void) const { return mTurnon; }

	void SetTurnon(bool flag, bool fireAnim = true);
	plugin_ui_control_frame_t GetTurnonFrame(void) const { return mTurnonFrame; }
	void SetTurnonFrame(const char * res);

	/*
	load & save & clone
	*/
    virtual void                        Load                    ( ui_data_control_t control );

	/*
	virtual
	*/
	virtual void						PerformLayout			( ui_vector_2_t client_sz );

protected:
	/*
	callback
	*/
	virtual void OnLoadProperty(void);

	/*
	event
	*/
	void ProcessMouseDrag(plugin_ui_touch_track_t track);
	void ProcessMouseRise(plugin_ui_touch_track_t track);
	void ProcessMouseClick(plugin_ui_touch_track_t track);

    static void on_mouse_rise(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void on_mouse_drag(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void on_mouse_click(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void setup(plugin_ui_control_meta_t meta);
    
	/*
	member
	*/
	bool mTurnon;
	RGUIButton * mMidBtn;
	ui_data_control_t mMidBtnTemplate;
	std::string mMidBtnTemplateName;
	plugin_ui_control_frame_t mTurnonFrame;

protected:
	/*
	hidden destructor
	*/
	virtual ~RGUISwitch(void);
    friend class RGUIControlRepo; 
};

#endif//__RGUISWITCH_H__
