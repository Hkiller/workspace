#ifndef DROW_UI_SCROLLABLE_H_INCLEDED
#define DROW_UI_SCROLLABLE_H_INCLEDED
#include <string>
#include "RGUIControl.h"

class RGUIScrollable : public RGUIControl {
public:
	RGUIScrollable();

	plugin_ui_control_t GetVScrollBarFrame(void) const;
	void SetVScrollBarFrame(const char * res);
    
	plugin_ui_control_t GetVScrollMidFrame(void) const;
	void SetVScrollMidFrame(const char * res);

	plugin_ui_control_t	GetHScrollBarFrame(void) const;
	void SetHScrollBarFrame(const char * res);
    
	plugin_ui_control_t GetHScrollMidFrame(void) const;
	void SetHScrollMidFrame(const char * res);

	uint8_t WasVScrollAutoHide( void ) const {	return mVScrollAutoHide; }
	void SetVScrollAutoHide( uint8_t flag ) { mVScrollAutoHide = flag; }
	uint8_t WasHScrollAutoHide( void ) const { return mHScrollAutoHide; }
	void SetHScrollAutoHide( uint8_t flag ) { mHScrollAutoHide = flag; }

	uint8_t WasVScrollSoft(void) const { return mVScrollSoft; }
	void SetVScrollSoft(uint8_t flag) { mVScrollSoft = flag; }
	uint8_t WasHScrollSoft(void) const { return mHScrollSoft; }
	void SetHScrollSoft(uint8_t flag) { mHScrollSoft = flag; }

	float GetHScrollRange(void) const { return mHScrollRange; }
	float GetVScrollRange(void) const { return mVScrollRange; }

	float GetHScrollValue(void) const { return GetScrollRealY(); }
	void SetHScrollValue(float value);

	float GetVScrollValue(void) const { return GetScrollRealX(); }
	void SetVScrollValue(float value);

	//void SetHScrollAlpha(float alpha) { mVScrollSoft = alpha; }

    void SetVScrollActivate(uint8_t flag) { mVScrollActivate = flag; }
    void SetHScrollActivate(uint8_t flag) { mHScrollActivate = flag; }

    using RGUIControl::Load;
    void Load(UI_CONTROL_SCROLL const & data);

    virtual void PerformLayout(ui_vector_2_t client_sz) = 0;
    virtual void layoutScrolls(void);

    RGUIControl * topButton(void) { return mTopBtn; }
    RGUIControl * bottomButton(void) { return mBottomBtn; }
    RGUIControl * leftButton(void) { return mLeftBtn; }
    RGUIControl * rightButton(void) { return mRightBtn; }

protected:
    static void on_layout(plugin_ui_control_t control, ui_vector_2_t client_sz);
    static void on_mouse_down(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void on_mouse_rise(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void on_mouse_drag(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void on_vscroll_changed(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void on_hscroll_changed(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void setup(plugin_ui_control_meta_t meta);

protected:
	virtual void UpdateSelf(float deltaTime);
	virtual void UpdateScrollRange(ui_vector_2_t client_sz);
    void OnLoadProperty();
    
	void SetHScrollValueAnim(int32_t value, int32_t frame, int32_t min, int32_t max);
	void SetVScrollValueAnim(int32_t value, int32_t frame, int32_t min, int32_t max);
	void SetHScrollAlphaAnim(void);
	void SetVScrollAlphaAnim(void);

    void stopVScrollAnim(void);
    void stopHScrollAnim(void);

    void updateVScrollBar(void);
    void updateVScrollMid(void);
    void updateHScrollBar(void);
    void updateHScrollMid(void);

    void ProcessMouseDown(plugin_ui_touch_track_t track);

    /*区域鼠标拖动 */
    void onAreaMouseMove(
        plugin_ui_touch_track_t track, ui_vector_2 const & down_pt, ui_vector_2 const & cur_pt, ui_vector_2 const & down_arg);
    void onAreaMouseRise(
        plugin_ui_touch_track_t track, ui_vector_2 const & down_pt, ui_vector_2 const & cur_pt, ui_vector_2 const & down_arg,
        ui_vector_2 const & speed);

    /*竖滑动块拖动 */
    void onVMidMouseMove(
        plugin_ui_touch_track_t track, ui_vector_2 const & down_pt, ui_vector_2 const & cur_pt, ui_vector_2 const & down_arg);
    void onVMidMouseRise(
        plugin_ui_touch_track_t track, ui_vector_2 const & down_pt, ui_vector_2 const & cur_pt, ui_vector_2 const & down_arg,
        ui_vector_2 const & speed);

protected:
	uint8_t mVScrollAutoHide;
	uint8_t	mHScrollAutoHide;
	uint8_t	mVScrollSoft;
	uint8_t	mHScrollSoft;
	uint8_t mVScrollActivate;
	uint8_t	mHScrollActivate;
	float mVScrollRange;
	float mHScrollRange;
	uint32_t m_btn_template_left_id;
	uint32_t m_btn_template_right_id;
	uint32_t m_btn_template_top_id;
	uint32_t m_btn_template_bottom_id;
    RGUIControl * mTopBtn;
    RGUIControl * mBottomBtn;
    RGUIControl * mLeftBtn;
    RGUIControl * mRightBtn;

    Drow::MouseProcessor * m_mouse_processor;
    
protected:
	virtual ~RGUIScrollable(void);
};

#endif
