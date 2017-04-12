#ifndef DROW_UI_CONTROL_SWIPER_H
#define DROW_UI_CONTROL_SWIPER_H
#include "RGUIControl.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUISwiper, ui_control_type_swiper);

class RGUISwiper : public RGUIControl {
public:
	int32_t GetCurrPageCount(void) const;
    int32_t GetCurrPage(void) const { return mCurrPage; }
	void SetCurrPage(int32_t page, bool fireAnim = true);
	
	plugin_ui_control_frame_t GetCurrPageFrame(void) const { return mCurrPageFrame; }
	void SetCurrPageFrame(const char * res);
    
	plugin_ui_control_frame_t GetGrayPageFrame(void) const { return mGrayPageFrame; }
	void SetGrayPageFrame(const char * res);

    virtual void Load(ui_data_control_t control);
	void PerformLayout(ui_vector_2_t client_sz);
    void SetAcceptRise(bool rise);

protected:
	/*
	method
	*/
	virtual void						UpdateSelf			( float deltaTime );
	virtual void						RenderTail			( ui_runtime_render_t ctx, ui_rect_t rect );

	/*
	event
	*/
	void ProcessDrag(plugin_ui_touch_track_t track);
	void ProcessRise(plugin_ui_touch_track_t track);

    static void on_mouse_rise(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void on_mouse_drag(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void on_show(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);    
    static void on_hide(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);    
    static void on_layout(plugin_ui_control_t control, ui_vector_2_t client_sz);    
    static void setup(plugin_ui_control_meta_t meta);

protected:
	/*
	member
	*/
	plugin_ui_control_frame_t mCurrPageFrame;
	plugin_ui_control_frame_t mGrayPageFrame;
	int32_t mCurrPage;
	float mDragDist;
	float mDragTime;
	float mStayTime;
    bool  mAcceptRise;
    
	RGUISwiper();
	virtual ~RGUISwiper(void);

    friend class RGUIControlRepo; 
};

#endif//__RGUISWIPER_H__
