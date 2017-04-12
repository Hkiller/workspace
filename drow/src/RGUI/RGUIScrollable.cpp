#include "cpe/utils/math_ex.h"
#include "gd/app/app_log.h"
#include "render/utils/ui_vector_2.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_touch_track.h"
#include "plugin/ui/plugin_ui_control_meta.h"
#include "plugin/ui/plugin_ui_control_frame.h"
#include "RGUIWindow.h"
#include "RGUIScrollable.h"
#include "RGUIControlScroll.hpp"
#include "RGUIMouseProcessor.hpp"

using namespace Drow;

RGUIScrollable::RGUIScrollable()
    : m_btn_template_left_id(0)
    , m_btn_template_right_id(0)
    , m_btn_template_top_id(0)
    , m_btn_template_bottom_id(0)
    , mTopBtn(NULL)
    , mBottomBtn(NULL)
    , mLeftBtn(NULL)
    , mRightBtn(NULL)
    , m_mouse_processor(NULL)
{
	mVScrollSoft = 		true;
	mHScrollSoft = 		true;
	mVScrollAutoHide = 	true;
	mHScrollAutoHide = 	true;
	mVScrollActivate	= true;
	mHScrollActivate	= true;
	mHScrollRange		= 0;
	mVScrollRange		= 0;
}

RGUIScrollable::~RGUIScrollable(void) {
    if (m_mouse_processor) {
        mem_free(allocrator(), m_mouse_processor);
        m_mouse_processor = NULL;
    }
}

void RGUIScrollable::Load(UI_CONTROL_SCROLL const & data) {
    mVScrollSoft = data.v_scroll_soft;
    mHScrollSoft = data.h_scroll_soft;
    mVScrollAutoHide = data.v_scroll_auto_hide;
    mHScrollAutoHide = data.h_scroll_auto_hide;
    m_btn_template_right_id = data.right_btn_template_id;
    m_btn_template_left_id = data.left_btn_template_id;
    m_btn_template_top_id = data.up_btn_template_id;
    m_btn_template_bottom_id = data.down_btn_template_id;

    ClearFrame(plugin_ui_control_frame_layer_tail, plugin_ui_control_frame_usage_normal, lock_aspect());

    plugin_ui_control_frame_t frame;

    frame = AddFrame(data.v_scroll_bar, plugin_ui_control_frame_layer_tail, plugin_ui_control_frame_usage_normal, lock_aspect());
    if (frame) plugin_ui_control_frame_set_name(frame, "v_bar");

    frame = AddFrame(data.v_scroll_mid, plugin_ui_control_frame_layer_tail, plugin_ui_control_frame_usage_normal, lock_aspect());
    if (frame) plugin_ui_control_frame_set_name(frame, "v_mid");

    frame = AddFrame(data.h_scroll_bar, plugin_ui_control_frame_layer_tail, plugin_ui_control_frame_usage_normal, lock_aspect());
    if (frame) plugin_ui_control_frame_set_name(frame, "h_bar");

    frame = AddFrame(data.h_scroll_mid, plugin_ui_control_frame_layer_tail, plugin_ui_control_frame_usage_normal, lock_aspect());
    if (frame) plugin_ui_control_frame_set_name(frame, "h_mid");
}

void RGUIScrollable::OnLoadProperty() {
	RGUIControl::OnLoadProperty();

    if ((mTopBtn = create(m_btn_template_top_id))) {
        AddChild(mTopBtn);
    }
    
    if ((mBottomBtn = create(m_btn_template_bottom_id))) {
        AddChild(mBottomBtn);
    }

    if ((mLeftBtn = create(m_btn_template_left_id))) {
        AddChild(mLeftBtn);
    }

    if ((mRightBtn = create(m_btn_template_right_id))) {
        AddChild(mRightBtn);
    }
}

void RGUIScrollable::SetHScrollValue(float value) {
    if (value < 0.0f) value = 0.0f;
    if (value > mHScrollRange) value = mHScrollRange;
    SetScrollRealX(value);
}

void RGUIScrollable::SetVScrollValue(float value) {
    if (value < 0.0f) value = 0.0f;
    if (value > mVScrollRange) value = mVScrollRange;
    SetScrollRealY(value);
}

void RGUIScrollable::ProcessMouseDown(plugin_ui_touch_track_t track) {
    stopVScrollAnim();
    stopHScrollAnim();
    if (m_mouse_processor) {
        delete m_mouse_processor;
        m_mouse_processor = NULL;
    }

    typedef DragProcessorT<RGUIScrollable> DragProcessor;
    
    ui_vector_2 local_pt = plugin_ui_control_pt_world_to_local(control(), plugin_ui_touch_track_cur_pt(track));
    
    plugin_ui_control_frame_t frame = findFrame(local_pt);
    if (frame) {
        if (strcmp(plugin_ui_control_frame_name(frame), "v_mid") == 0) {
            m_mouse_processor = new DragProcessor(
                *this, track, *plugin_ui_control_frame_local_pos(frame),
                &RGUIScrollable::onVMidMouseMove, &RGUIScrollable::onVMidMouseRise);
        }
        else if (strcmp(plugin_ui_control_frame_name(frame), "h_mid") == 0) {
            //way = ScrollDragHMid;
        }
    }

    if (m_mouse_processor == NULL) {
        m_mouse_processor = new DragProcessor(
            *this, track, *plugin_ui_control_scroll(control()),
            &RGUIScrollable::onAreaMouseMove, &RGUIScrollable::onAreaMouseRise);
    }
}

void RGUIScrollable::on_vscroll_changed(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
	if (ctx == from_control) {
        RGUIScrollable * c = cast<RGUIScrollable>((plugin_ui_control_t)ctx);
        c->updateVScrollMid();
	}
}

void RGUIScrollable::on_hscroll_changed(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    if (ctx == from_control) {
        RGUIScrollable * c = cast<RGUIScrollable>((plugin_ui_control_t)ctx);
        c->updateHScrollMid();
    }
}

void RGUIScrollable::UpdateSelf(float deltaTime) {
	RGUIControl::UpdateSelf(deltaTime);
}

void RGUIScrollable::UpdateScrollRange(ui_vector_2_t client_sz) {
	ui_rect renderRT = GetRenderTextRT();

	//RRect clientRT = GetClientTextRT();
	ui_rect rt = UI_RECT_INITLIZER(0, 0, client_sz->x, client_sz->y);
    ui_rect pd = GetClientRealPD();
	ui_rect clientRT = UI_RECT_INITLIZER(rt.lt.x + pd.lt.x, rt.lt.y + pd.lt.y, rt.rb.x - pd.rb.x, rt.rb.y - pd.rb.y);

	mHScrollRange = ui_rect_width(&clientRT) - ui_rect_width(&renderRT);
	mVScrollRange = ui_rect_height(&clientRT) - ui_rect_height(&renderRT);
    // APP_CTX_ERROR(app(), "%s.%s: UpdateScrollRange", plugin_ui_page_name(page()), plugin_ui_control_name(control()));
    // APP_CTX_ERROR(app(), "renderRT=(%f,%f)-(%f,%f)", renderRT.lt.x, renderRT.lt.y,renderRT.rb.x, renderRT.rb.y);
    // APP_CTX_ERROR(app(), "clientRT=(%f,%f)-(%f,%f)", clientRT.lt.x, clientRT.lt.y,clientRT.rb.x, clientRT.rb.y);
}

void RGUIScrollable::SetHScrollValueAnim( int32_t value, int32_t frame, int32_t min, int32_t max ) {
	if (value == GetScrollRealX())
		return;

	int32_t destFrame = frame;
	int32_t destValue = value;

	if (value < min || value > max) {
		int32_t destDelta = GetScrollRealX() - value;
		int32_t destRange = value < min ? GetScrollRealX() : GetScrollRealX() - max;

		destFrame = (int32_t)(asin((float)destRange / (float)destDelta) / M_PI_2 * frame) + 5;
		destDelta = (int32_t)(destDelta * sin(M_PI_2 * (float)(destFrame) / (float)frame));
		destValue = GetScrollRealX() - destDelta;
	}

    stopHScrollAnim();

    createAnim<Drow::ControlScroll>(lock_aspect())
        .setGuardDone(0)
        .setTakeTime((float)frame / 60)
        .setTargetY(destValue)
        .start();
}

void RGUIScrollable::SetVScrollValueAnim(int32_t value, int32_t frame, int32_t min, int32_t max) {
	if (value == GetScrollRealY())
		return;

	int32_t destFrame = frame;
	int32_t destValue = value;

	if (value < min || value > max) {
		int32_t destDelta = GetScrollRealY() - value;
		int32_t destRange = value < min ? GetScrollRealY() : GetScrollRealY() - max;

		destFrame = (int32_t)(asin((float)destRange / (float)destDelta) / M_PI_2 * frame) + 5;
		destDelta = (int32_t)(destDelta * sin(M_PI_2 * (float)(destFrame) / (float)frame));
		destValue = GetScrollRealY() - destDelta;
	}

    stopVScrollAnim();

    Drow::ControlScroll & scroll_anim = createAnim<Drow::ControlScroll>(lock_aspect())
        .setGuardDone(0)
        .setTakeTime((float)frame / 60)
        .setTargetY(destValue);

    // if (mVScrollAutoHide) {
    //     scroll_anim.onComplete(*this, &RGUIScrollable::SetVScrollAlphaAnim);
    // }
    
    scroll_anim.start();
}

void RGUIScrollable::SetHScrollAlphaAnim	( void ) {
	// mHScrollAnimCtrl->Clear();
	// mHScrollAnimCtrl->AddFrameKey(0);
	// mHScrollAnimCtrl->AddAlphaKey(1.0f);
	// mHScrollAnimCtrl->AddFrameKey(15);
	// mHScrollAnimCtrl->AddAlphaKey(0.0f);
	// mHScrollAnimCtrl->SetPlay(true);
}

void RGUIScrollable::SetVScrollAlphaAnim	( void ) {
	// mVScrollAnimCtrl->AddFrameKey(0);
	// mVScrollAnimCtrl->AddAlphaKey(1.0f);
	// mVScrollAnimCtrl->AddFrameKey(15);
	// mVScrollAnimCtrl->AddAlphaKey(0.0f);
	// mVScrollAnimCtrl->SetPlay(true);
}

void RGUIScrollable::stopVScrollAnim(void) {
    plugin_ui_animation_it anim_it;
    plugin_ui_control_animations(control(), &anim_it, ControlScroll::TYPE_NAME, lock_aspect());

    while(plugin_ui_animation_t anim = plugin_ui_animation_it_next(&anim_it)) {
        if (ControlScroll::cast(anim)->processX()) {
            plugin_ui_animation_free(anim);
            return;
        }
    }
}

void RGUIScrollable::stopHScrollAnim(void) {
    plugin_ui_animation_it anim_it;
    plugin_ui_control_animations(control(), &anim_it, ControlScroll::TYPE_NAME, lock_aspect());

    while(plugin_ui_animation_t anim = plugin_ui_animation_it_next(&anim_it)) {
        if (ControlScroll::cast(anim)->processY()) {
            plugin_ui_animation_free(anim);
            return;
        }
    }
}

void RGUIScrollable::updateVScrollBar(void) {
    ui_vector_2_t client_sz = plugin_ui_control_real_sz_no_scale(control());

    /*放置上下按钮 */
    ui_vector_2 top_btn_size = UI_VECTOR_2_ZERO;
    if(mTopBtn) {
        top_btn_size = plugin_ui_control_calc_child_real_sz_no_scale(control(), plugin_ui_control_data_src(mTopBtn->control()));
        ui_vector_2 pt = UI_VECTOR_2_INITLIZER(client_sz->x - top_btn_size.x, 0.0f);
        plugin_ui_control_set_render_pt_by_real(mTopBtn->control(), &pt); 
    }
    ui_vector_2 bottom_btn_size = UI_VECTOR_2_ZERO;
    if(mBottomBtn) {
        bottom_btn_size = plugin_ui_control_calc_child_real_sz_no_scale(control(), plugin_ui_control_data_src(mBottomBtn->control()));
        ui_vector_2 pt = UI_VECTOR_2_INITLIZER(client_sz->x - bottom_btn_size.x, client_sz->y - bottom_btn_size.y);
        plugin_ui_control_set_render_pt_by_real(mBottomBtn->control(), &pt); 
    }

	plugin_ui_control_frame_t bar = findFrame("v_bar");
    if (bar == NULL) return;
    
    ui_rect bar_rect;
    if (frame_rect_or_static(bar, bar_rect) != 0) return;
    float bar_width = ui_rect_width(&bar_rect);

    ui_rect local_rect;
    local_rect.lt.x = client_sz->x - bar_width;
    local_rect.rb.x = client_sz->x;
    local_rect.lt.y = top_btn_size.y;
    local_rect.rb.y = cpe_max(top_btn_size.y, client_sz->y - bottom_btn_size.y);

    // printf("xxxxx: size=(%f,%f), v_bar=(%f,%f)-(%f,%f)\n",
    //        client_sz->x, client_sz->y,
    //        local_rect.lt.x, local_rect.lt.y, local_rect.rb.x, local_rect.rb.y);
    
    plugin_ui_control_frame_set_local_rect(bar, &local_rect);
    plugin_ui_control_frame_set_visible(bar, local_rect.lt.y < local_rect.rb.y ? 1 : 0);
}

void RGUIScrollable::updateVScrollMid(void) {
    plugin_ui_control_frame_t mid = findFrame("v_mid");
    if (mid == NULL) return;
    
    ui_vector_2_t client_sz = plugin_ui_control_real_sz_no_scale(control());

    ui_rect mid_rect;
	if (frame_rect_or_static(mid, mid_rect) != 0) return;
    float mid_width = ui_rect_width(&mid_rect);
    float mid_size = (client_sz->y / (client_sz->y + mVScrollRange)) * client_sz->y;

    float bar_y_min, bar_y_max;
    if (plugin_ui_control_frame_t bar = findFrame("v_bar")) {
        bar_y_min = plugin_ui_control_frame_local_pos(bar)->y;
        bar_y_max = bar_y_min + plugin_ui_control_frame_render_size(bar)->y;
    }
    else {
        bar_y_min = 0.0f;
        bar_y_max = client_sz->y;
    }
    
    ui_rect local_rect;
    local_rect.lt.x = client_sz->x - mid_width;
    local_rect.rb.x = client_sz->x;
    local_rect.lt.y = bar_y_min;
    if (mVScrollRange > 0) {
        float left_size = client_sz->y - mid_size;
        float percent = GetScrollRealY() / mVScrollRange;
        local_rect.lt.y += percent * left_size;
    }
    
    local_rect.rb.y = local_rect.lt.y + mid_size;

    plugin_ui_control_frame_set_local_rect(mid, &local_rect);
}

void RGUIScrollable::onAreaMouseMove(plugin_ui_touch_track_t track, ui_vector_2 const & down_pt, ui_vector_2 const & cur_pt, ui_vector_2 const & down_scroll) {
    uint8_t is_processed = 0;
    
	if (mHScrollActivate) {
        SetHScrollValue(down_scroll.x - cur_pt.x + down_pt.x);
        is_processed = 1;
	}

	if (mVScrollActivate) {
        SetVScrollValue(down_scroll.y - cur_pt.y + down_pt.y);
	}

    if (is_processed) plugin_ui_touch_track_set_process_control(track, control());
}

void RGUIScrollable::onAreaMouseRise(plugin_ui_touch_track_t track, ui_vector_2 const & down_pt, ui_vector_2 const & cur_pt, ui_vector_2 const & down_arg, ui_vector_2 const & speed) {
    // if (mMouseStayTime < 0.1f && mMouseDragTime > 0.0f) {
    //     if (mHScrollActivate) {
    //         if (GetScrollRealX() < 0 || GetScrollRealX() > mHScrollRange) {
    //             if (GetScrollRealX() < 0)				SetHScrollValueAnim(0,				15, 0, mHScrollRange);
    //             if (GetScrollRealX() > mHScrollRange)	SetHScrollValueAnim(mHScrollRange,	15, 0, mHScrollRange);
    //         }
    //         else {
    //             int32_t delta = (int32_t)(mMouseDragDist.x / mMouseDragTime) / 2;
    //             int32_t value = GetScrollRealX() - delta;
    //             SetHScrollValueAnim(value, 60, 0, mHScrollRange);
    //         }
    //     }

    //     if (mVScrollActivate) {
    //         if (GetScrollRealY() < 0 || GetScrollRealY() > mVScrollRange) {
    //             if (GetScrollRealY() < 0)				SetVScrollValueAnim(0,				15, 0, mVScrollRange);
    //             if (GetScrollRealY() > mVScrollRange)	SetVScrollValueAnim(mVScrollRange,	15, 0, mVScrollRange);
    //         }
    //         else {
    //             int32_t delta = (int32_t)(mMouseDragDist.y / mMouseDragTime) / 2;
    //             int32_t value = GetScrollRealY() - delta;
    //             SetVScrollValueAnim(value, 60, 0, mVScrollRange);
    //         }
    //     }
    // }
}

void RGUIScrollable::onVMidMouseMove(plugin_ui_touch_track_t track, ui_vector_2 const & down_pt, ui_vector_2 const & cur_pt, ui_vector_2 const & down_scroll) {
    ui_vector_2_t client_sz = plugin_ui_control_real_sz_no_scale(control());

    plugin_ui_control_frame_t mid = findFrame("v_mid");
    if (mid == NULL) return;

    float range = client_sz->y - plugin_ui_control_frame_render_size(mid)->y;
    float pos = down_scroll.y - down_pt.y + cur_pt.y;
    pos = cpe_limit_in_range(pos, 0, range);

    float percent = pos / range;
    
    SetVScrollValue(mVScrollRange * percent);

    plugin_ui_touch_track_set_process_control(track, control());
}

void RGUIScrollable::onVMidMouseRise(plugin_ui_touch_track_t track, ui_vector_2 const & down_pt, ui_vector_2 const & cur_pt, ui_vector_2 const & down_arg, ui_vector_2 const & speed) {
}

void RGUIScrollable::updateHScrollBar(void) {
}

void RGUIScrollable::updateHScrollMid(void) {
}

void RGUIScrollable::layoutScrolls(void) {
    updateVScrollBar();
    updateVScrollMid();
    updateHScrollBar();
    updateHScrollMid();
}

void RGUIScrollable::on_layout(plugin_ui_control_t control, ui_vector_2_t client_sz) {
    RGUIScrollable * scroll = cast<RGUIScrollable>(control);
	scroll->PerformLayout(client_sz);
	scroll->UpdateScrollRange(client_sz);
    scroll->layoutScrolls();
}

void RGUIScrollable::on_mouse_down(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    RGUIControl::on_mouse_down(ctx, from_control, event);

    RGUIScrollable * scroll = cast<RGUIScrollable>((plugin_ui_control_t)ctx);
    scroll->ProcessMouseDown(plugin_ui_control_touch_track(from_control));
}

void RGUIScrollable::on_mouse_rise(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    plugin_ui_control_t control = (plugin_ui_control_t)ctx;
    plugin_ui_touch_track_t track = plugin_ui_control_touch_track(from_control);
    
	if (plugin_ui_touch_track_process_control(track) && plugin_ui_touch_track_process_control(track) != control) return;

    RGUIScrollable * scroll = cast<RGUIScrollable>(control);
    if (scroll->m_mouse_processor == NULL) return;

    scroll->m_mouse_processor->rise(track);
}

void RGUIScrollable::on_mouse_drag(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    plugin_ui_control_t control = (plugin_ui_control_t)ctx;
    plugin_ui_touch_track_t track = plugin_ui_control_touch_track(from_control);
    
	if (plugin_ui_touch_track_process_control(track) && plugin_ui_touch_track_process_control(track) != control) return;

    RGUIScrollable * scroll = cast<RGUIScrollable>(control);
    if (scroll->m_mouse_processor == NULL) return;

    scroll->m_mouse_processor->move(track);
}

void RGUIScrollable::setup(plugin_ui_control_meta_t meta) {
    RGUIControl::setup(meta);

    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_down, plugin_ui_event_scope_all, on_mouse_down);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_move, plugin_ui_event_scope_all, on_mouse_drag);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_up, plugin_ui_event_scope_all, on_mouse_rise);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_vscroll_changed, plugin_ui_event_scope_self, on_vscroll_changed);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_hscroll_changed, plugin_ui_event_scope_self, on_hscroll_changed);

    plugin_ui_control_meta_set_layout(meta, on_layout);
}
