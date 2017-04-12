#include "cpe/pal/pal_math.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_control_meta.h"
#include "plugin/ui/plugin_ui_control_frame.h"
#include "plugin/ui/plugin_ui_touch_track.h"
#include "RGUIWindow.h"
#include "RGUIButton.h"
#include "RGUISlider.h"

/*
constructor
*/
RGUISlider::RGUISlider()
    : mSliderBeginPercent(-1.0f)
    , mSliderBeginValue(0)
    , mSliderPrePercent(-1.0f)
{
	mMidBtnTemplateName = 	"";
	mStatusFrame = 		NULL;
	mSliderRange = 		10;

	mSliderValue	= 0;
	mMidBtn			= create<RGUIButton>();
	mMidBtnTemplate = NULL;
	AddChild(mMidBtn);
}

RGUISlider::~RGUISlider(void) {
    if (mStatusFrame) {
        plugin_ui_control_frame_free(mStatusFrame);
        mStatusFrame = NULL;
    }
}

/*
method
*/
void RGUISlider::SetSliderRange(uint32_t range) {
	if (mSliderRange != range) {
		mSliderRange  = range;
		UpdateMidByPos();
	}
}

void	RGUISlider::SetSliderValue			( uint32_t value )
{
	if (mSliderValue != cpe_min((uint32_t)mSliderRange, value))
	{
		mSliderValue  = cpe_min((uint32_t)mSliderRange, value);
		UpdateMidByPos();

        plugin_ui_control_dispatch_event(
            control(), control(),
            plugin_ui_event_slider_changed, plugin_ui_event_dispatch_to_self_and_parent);
	}
}

/*
load & save & clone
*/
void RGUISlider::Load( ui_data_control_t control ) {
    RGUIControl::Load(control);

    if (type() == ui_control_type_slider) {
        UI_CONTROL const & data = *ui_data_control_data(control);
        if (mStatusFrame) {
            plugin_ui_control_frame_free(mStatusFrame);
        }
        mStatusFrame = plugin_ui_control_frame_create_by_def(this->control(), plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_normal, &data.data.slider.status_frame);
        mSliderRange = data.data.slider.slider_range;
        mMidBtnTemplateName = ui_data_control_msg(control, data.data.slider.mid_btn_template_id);
    }
}

/*
virtual
*/
void RGUISlider::PerformLayout(ui_vector_2_t client_sz)
{
    client_sz->x = client_sz->y = 0.0f;

	mMidBtn->SetAlignVert(ui_align_mode_vert_center);
	UpdateMidByPos();
}

/*
callback
*/
void RGUISlider::OnLoadProperty()
{
	RGUIControl::OnLoadProperty();
    
	//得到模版指针  
	mMidBtnTemplate = GetTemplate(mMidBtnTemplateName.c_str());
	if (mMidBtnTemplate) {
		mMidBtn->SetTemplateLinkCtrl(mMidBtnTemplate);
		mMidBtn->SetParentClip(true);
	}
}

/*
event
*/
void RGUISlider::ProcessDrag(plugin_ui_touch_track_t track) {
	if (plugin_ui_touch_track_process_control(track) && plugin_ui_touch_track_process_control(track) != control()) {
		return;
    }
    
    if (mSliderBeginPercent < 0.0f) {
        mSliderBeginPercent = GetSliderPercent();
        mSliderBeginValue = mSliderValue;
        mSliderPrePercent = mSliderBeginPercent;
        mSliderMovingWay = Moving_Idle;
    }

    ui_rect pd  = GetClientRealPD();
    float  min =				       (float)pd.lt.x;
    float  max = GetRenderRealSZ().w - (float)pd.rb.x - mMidBtn->GetRenderRealW();
       
    float  x   = mMidBtn->GetRenderRealX() + plugin_ui_touch_track_cur_pt(track)->x - plugin_ui_touch_track_last_pt(track)->x;
    if (x < min)	x = min;
    if (x > max)	x = max;
    mMidBtn->SetRenderRealX(x);
    mMidBtn->checkUpdatCache();

    /*检测滑动方向 */
    float curPercent = GetSliderPercent();
    if (fabs(curPercent - mSliderPrePercent) > 0.01) {
        mSliderMovingWay = curPercent < mSliderPrePercent ? Moving_Small : Moving_Bigger;
        mSliderPrePercent = curPercent;
    }

    //更新数值 
    UpdatePosByMid(false);

    //滚动事件处理
    plugin_ui_touch_track_set_process_control(track, control());
}

void RGUISlider::ProcessRise(plugin_ui_touch_track_t track) {
	if (plugin_ui_touch_track_process_control(track) && plugin_ui_touch_track_process_control(track) != control()) {
		return;
    }
    
    UpdatePosByMid(true);
    UpdateMidByPos();

    mSliderBeginPercent = -1;
    mSliderBeginValue = 0;
    mSliderPrePercent = -1;
    mSliderMovingWay = Moving_Idle;
}

void	RGUISlider::UpdateMidByPos			( void )
{
	float r = (mSliderRange == 0) ? 0.0f : (float)mSliderValue / (float)mSliderRange;
    ui_rect textRt = GetRenderTextRT();
	float x = (ui_rect_width(&textRt) - mMidBtn->GetRenderRealW()) * r + GetClientRealPD().lt.x;
	mMidBtn->SetRenderRealX(ui_pixel_aligned(x));
    mMidBtn->checkUpdatCache();
}

float RGUISlider::GetSliderPercent(void) const {
    ui_rect textRt = GetRenderTextRT();
	return (mMidBtn->GetRenderRealX() - (float)GetClientRealPD().lt.x) / (ui_rect_width(&textRt) - mMidBtn->GetRenderRealW());
}

void	RGUISlider::UpdatePosByMid			( bool smartAdj )
{
	float r = GetSliderPercent();
	float f = r * (float)mSliderRange;
    uint32_t v = (uint32_t)ui_pixel_aligned(f);

    v = cpe_min((uint32_t)mSliderRange, v);
    if (smartAdj && v == mSliderBeginValue) {
        if (r > mSliderBeginPercent && mSliderMovingWay == Moving_Bigger) {
            if (v < mSliderRange) v += 1;
        }
        else if (r < mSliderBeginPercent && mSliderMovingWay == Moving_Small) {
            if (v > 0) v -= 1;
        }
    }
    
    if (mSliderValue != v) {
        mSliderValue  = v;
        
        plugin_ui_control_dispatch_event(
            control(), control(),
            plugin_ui_event_slider_changed, plugin_ui_event_dispatch_to_self_and_parent);
    }
}

void RGUISlider::on_mouse_rise(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    RGUIControl::on_mouse_rise(ctx, from_control, event);

    RGUISlider * slider = cast<RGUISlider>((plugin_ui_control_t)ctx);

    plugin_ui_touch_track_t track = plugin_ui_control_touch_track(from_control);
    
	if (from_control == slider->mMidBtn->control() && plugin_ui_touch_track_is_horz_move(track)) {
        slider->ProcessRise(track);
    }    
}

void RGUISlider::on_mouse_drag(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    RGUISlider * slider = cast<RGUISlider>((plugin_ui_control_t)ctx);

    plugin_ui_touch_track_t track = plugin_ui_control_touch_track(from_control);
    
	if (from_control == slider->mMidBtn->control() && plugin_ui_touch_track_is_horz_move(track)) {
        slider->ProcessDrag(track);
    }    
}

void RGUISlider::setup(plugin_ui_control_meta_t meta) {
    RGUIControl::setup(meta);

    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_move, plugin_ui_event_scope_childs, on_mouse_drag);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_up, plugin_ui_event_scope_childs, on_mouse_rise);
}
