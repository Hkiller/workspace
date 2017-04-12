#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_control_meta.h"
#include "plugin/ui/plugin_ui_control_frame.h"
#include "plugin/ui/plugin_ui_touch_track.h"
#include "RGUIWindow.h"
#include "RGUIButton.h"
#include "RGUISwitch.h"

/*
constructor
*/
RGUISwitch::RGUISwitch() {
	mMidBtnTemplateName = 	"";
	mTurnonFrame = NULL;

	mTurnon			= false;
	mMidBtn			= create<RGUIButton>();
	mMidBtnTemplate = NULL;
	AddChild(mMidBtn);
}

RGUISwitch::~RGUISwitch() {
    if (mTurnonFrame) {
        plugin_ui_control_frame_free(mTurnonFrame);
        mTurnonFrame = NULL;
    }
}

/*
method
*/
void	RGUISwitch::SetTurnon				( bool flag, bool fireAnim )
{
	if (mTurnon != flag)
	{
		mTurnon  = flag;

        plugin_ui_control_dispatch_event(
            control(), control(),
            plugin_ui_event_switch_changed, plugin_ui_event_dispatch_to_self_and_parent);
	}

	//计算位置 
    ui_rect pd  = GetClientRealPD();
	float  min =					   (float)pd.lt.x;
	float  max = GetRenderRealSZ().w - (float)pd.rb.x - mMidBtn->GetRenderRealW();

	RVector2     pt;
	if (mTurnon)  pt = RVector2(max, mMidBtn->GetRenderRealY());
	else		  pt = RVector2(min, mMidBtn->GetRenderRealY());

	RGUIUnitVec2 ut = RGUIUnitVec2::ToUnit(mMidBtn->WasAcceptPTLS(), GetRenderRealSZ(), GetEditorRealSZ(), pt);
	if (ut == mMidBtn->GetRenderUnitPT())
		return;

	//触发动画 
	if (fireAnim)
	{
        /*
		RGUIActorKeyData ak;
		ak.frameKeyVec.push_back(0);
		ak.frameKeyVec.push_back(5);
		ak.transKeyVec.push_back(mMidBtn->GetRenderUnitPT());
		ak.transKeyVec.push_back(ut);

		mMidBtn->GetMoveAnimCtrl()->Clear();
		mMidBtn->GetMoveAnimCtrl()->SetActorKeyVec(ak);
		mMidBtn->GetMoveAnimCtrl()->SetSoft(true);
		mMidBtn->GetMoveAnimCtrl()->SetPlay(true);
         */
	}
	else
	{
		mMidBtn->SetRenderUnitPT(ut);
	}
}

/*
load & save & clone
*/
void RGUISwitch::Load( ui_data_control_t control ) {
    RGUIControl::Load(control);

    if (type() == ui_control_type_slider) {
        UI_CONTROL const & data = *ui_data_control_data(control);
        if (mTurnonFrame) {
            plugin_ui_control_frame_free(mTurnonFrame);
        }
        mTurnonFrame = plugin_ui_control_frame_create_by_def(this->control(), plugin_ui_control_frame_layer_tail, plugin_ui_control_frame_usage_normal, &data.data.switcher.turnon_frame);
        mMidBtnTemplateName = ui_data_control_msg(control, data.data.switcher.mid_btn_template_id);
        //plugin_ui_control_set_layer_render_before(this->control(), plugin_ui_control_frame_layer_tail, 1);
    }
}

/*
virtual
*/
void RGUISwitch::PerformLayout(ui_vector_2_t client_sz)
{
    client_sz->x = client_sz->y = 0.0f;
    
	//计算位置 
    ui_rect pd  = GetClientRealPD();
	float  min =					   (float)pd.lt.x;
	float  max = GetRenderRealSZ().w - (float)pd.rb.x - mMidBtn->GetRenderRealW();

	RVector2    pt;
	if (mTurnon) pt = RVector2(max, mMidBtn->GetRenderRealY());
	else		 pt = RVector2(min, mMidBtn->GetRenderRealY());

	mMidBtn->SetRenderRealPT(pt);
	mMidBtn->SetAlignVert(ui_align_mode_vert_center);
}

/*
callback
*/
void RGUISwitch::OnLoadProperty() {
	RGUIControl::OnLoadProperty();
	//得到模版指针 
	mMidBtnTemplate = GetTemplate(mMidBtnTemplateName.c_str());
	if (mMidBtnTemplate)
	{
		mMidBtn->SetTemplateLinkCtrl(mMidBtnTemplate);
		mMidBtn->SetParentClip(true);
	}
}

/*
event
*/
void RGUISwitch::ProcessMouseDrag(plugin_ui_touch_track_t track) {
    if (plugin_ui_touch_track_is_horz_move(track)) {
        ui_rect pd  = GetClientRealPD();
		float  min =					   (float)pd.lt.x;
		float  max = GetRenderRealSZ().w - (float)pd.rb.x - mMidBtn->GetRenderRealW();

		float x   = mMidBtn->GetRenderRealX() + plugin_ui_touch_track_cur_pt(track)->x - plugin_ui_touch_track_last_pt(track)->x;
		if (x < min)	x = min;
		if (x > max)	x = max;
		mMidBtn->SetRenderRealX(x);

		//滚动事件处理
        plugin_ui_touch_track_set_process_control(track, control());
	}
}

void RGUISwitch::ProcessMouseRise(plugin_ui_touch_track_t track) {
    if (plugin_ui_touch_track_is_horz_move(track)) {
		ui_rect rt = GetRenderTextRT();
		int32_t  x  = (int32_t)(mMidBtn->GetRenderRealX() + mMidBtn->GetRenderRealW() / 2.0f);
		int32_t  hf = rt.lt.x + ui_rect_width(&rt) / 2;
		SetTurnon(x > hf);
	}
}

void RGUISwitch::ProcessMouseClick(plugin_ui_touch_track_t track) {
	ui_rect rt = GetRenderTextRTAbs();
	if (!ui_rect_is_contain_pt(&rt, plugin_ui_touch_track_cur_pt(track)))
		return;

	/*if (!mMidBtn->GetMoveAnimCtrl()->WasPlay())*/
		SetTurnon(!mTurnon);
}

void RGUISwitch::on_mouse_rise(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    RGUIControl::on_mouse_rise(ctx, from_control, event);

    RGUISwitch * s = cast<RGUISwitch>(from_control);

    s->ProcessMouseRise(plugin_ui_control_touch_track(from_control));
}

void RGUISwitch::on_mouse_drag(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    RGUISwitch * s = cast<RGUISwitch>(from_control);

    s->ProcessMouseDrag(plugin_ui_control_touch_track(from_control));
}

void RGUISwitch::on_mouse_click(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    RGUIControl::on_mouse_click(ctx, from_control, event);

    RGUISwitch * s = cast<RGUISwitch>(from_control);

    s->ProcessMouseClick(plugin_ui_control_touch_track(from_control));
}

void RGUISwitch::setup(plugin_ui_control_meta_t meta) {
    RGUIControl::setup(meta);

    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_up, plugin_ui_event_scope_childs, on_mouse_rise);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_move, plugin_ui_event_scope_childs, on_mouse_drag);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_click, plugin_ui_event_scope_childs, on_mouse_click);
}    

