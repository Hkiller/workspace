#ifndef DROW_UI_CONTROL_SLIDER_H
#define DROW_UI_CONTROL_SLIDER_H
#include <string>
#include "RGUIControl.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUISlider, ui_control_type_slider);

class RGUISlider : public RGUIControl {
public:
	/*
	constructor
	*/
	RGUISlider();

	/*
	method
	*/
	plugin_ui_control_frame_t GetStatusFrame(void) const { return mStatusFrame; }
	void SetStatusFrame( const char * res);

	uint32_t GetSliderRange(void) const { return mSliderRange; }
	void SetSliderRange(uint32_t range);
    
	uint32_t GetSliderValue(void) const { 	return mSliderValue; }
	void SetSliderValue(uint32_t value);
    float GetSliderPercent(void) const;

	RGUIButton * midButton(void) { return mMidBtn; }
    
	/*
	load & save
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
    void ProcessDrag(plugin_ui_touch_track_t track);
    void ProcessRise(plugin_ui_touch_track_t track);    

	virtual void						UpdateMidByPos			( void );
	virtual void						UpdatePosByMid			( bool smartAdj );

    static void on_mouse_rise(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void on_mouse_drag(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void setup(plugin_ui_control_meta_t meta);
    static void anim_resize_init(plugin_ui_module_t ui_module);
    
	/*
	member
	*/
	RGUIButton*						mMidBtn;
	ui_data_control_t						mMidBtnTemplate;
	std::string		mMidBtnTemplateName;
	plugin_ui_control_frame_t mStatusFrame;
	uint32_t			mSliderRange;
	uint32_t								mSliderValue;
    float                               mSliderBeginPercent;
    uint32_t                               mSliderBeginValue;
    float                               mSliderPrePercent;

    enum MovingWay {
        Moving_Idle,
        Moving_Bigger,
        Moving_Small
    };

    MovingWay                            mSliderMovingWay;

protected:
	/*
	hidden destructor
	*/
	virtual ~RGUISlider(void);
    friend class RGUIControlRepo; 
};

#endif//__RGUISLIDER_H__
