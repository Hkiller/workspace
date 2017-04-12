#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_control_meta.h"
#include "plugin/ui/plugin_ui_control_frame.h"
#include "plugin/ui/plugin_ui_touch_track.h"
#include "RGUIWindow.h"
#include "RGUISwiper.h"

/*
constructor
*/
RGUISwiper::RGUISwiper() {
	mCurrPageFrame = NULL;
	mGrayPageFrame = NULL;

	mCurrPage = 0;
	mDragDist = 0.0f;
	mDragTime = 0.0f;
	mStayTime = 0.0f;
    mAcceptRise = true;
}

RGUISwiper::~RGUISwiper() {
    if (mCurrPageFrame) {
        plugin_ui_control_frame_free(mCurrPageFrame);
        mCurrPageFrame = NULL;
    }

    if (mGrayPageFrame) {
        plugin_ui_control_frame_free(mGrayPageFrame);
        mGrayPageFrame = NULL;
    }
}

/*
method
*/
void RGUISwiper::SetCurrPage(int32_t page, bool fireAnim) {
	if (mCurrPage != page) {
		mCurrPage  = page;

        plugin_ui_control_dispatch_event(
            control(), control(),
            plugin_ui_event_swiper_changed, plugin_ui_event_dispatch_to_self_and_parent);
	}

	if (fireAnim) {
        RVector2 curr = GetRenderRealSZ();
        RVector2 orig = GetEditorRealSZ();
		ui_rect    rt   = GetRenderTextRT();
        ui_rect    pd   = GetClientRealPD();
		int32_t     w    = ui_rect_width(&rt);

		RVector2 pt = RVector2((float)(pd.lt.x - mCurrPage * w), (float)pd.lt.y);

        plugin_ui_control_it child_it;
        plugin_ui_control_childs(control(), &child_it);

        for(plugin_ui_control_t child = plugin_ui_control_it_next(&child_it);
            child;
            child = plugin_ui_control_it_next(&child_it))
        {
            RGUIControl * c = (RGUIControl*)plugin_ui_control_product(child);

			RGUIUnitVec2 ut = RGUIUnitVec2::ToUnit(c->WasAcceptPTLS(), curr, orig, pt);
			if (!c->WasVisible())
				continue;

            /*
			RGUIActorKeyData ak;
			ak.frameKeyVec.push_back(0);
			ak.frameKeyVec.push_back(10);
			ak.transKeyVec.push_back(c->GetRenderUnitPT());
			ak.transKeyVec.push_back(ut);

			c->GetMoveAnimCtrl()->Clear();
			c->GetMoveAnimCtrl()->SetActorKeyVec(ak);
			c->GetMoveAnimCtrl()->SetSoft(true);
			c->GetMoveAnimCtrl()->SetPlayHandler(new RGUIEventHandlerMem<RGUISwiper>(this, &RGUISwiper::OnPlaySwiperAnim));
			c->GetMoveAnimCtrl()->SetStopHandler(new RGUIEventHandlerMem<RGUISwiper>(this, &RGUISwiper::OnStopSwiperAnim));
			c->GetMoveAnimCtrl()->SetPlay(true);
             */

			pt.x += (float)w;
		}
	}
	else
	{
		//Invalid();
	}
}

void RGUISwiper::SetGrayPageFrame(const char * res) {
	// if (mGrayPageFrame != frame)
	// {
	// 	mGrayPageFrame  = frame;
	// }
}

void RGUISwiper::SetAcceptRise(bool rise) {
    mAcceptRise = rise;
}
int32_t RGUISwiper::GetCurrPageCount(void) const {
	int32_t  count = 0;

    plugin_ui_control_it child_it;
    plugin_ui_control_childs(control(), &child_it);

    for(plugin_ui_control_t child = plugin_ui_control_it_next(&child_it);
        child;
        child = plugin_ui_control_it_next(&child_it))
    {
        RGUIControl * c = (RGUIControl*)plugin_ui_control_product(child);
		if (c->WasVisible()) {
			++count;
        }
    }

	return count;
}


/*
load & save & clone
*/
void    RGUISwiper::Load( ui_data_control_t control ) {
    RGUIControl::Load( control );

    if (type() == ui_control_type_swiper) {
        UI_CONTROL const & data = *ui_data_control_data(control);
        if (mCurrPageFrame) {
            plugin_ui_control_frame_free(mCurrPageFrame);
        }
        mCurrPageFrame = plugin_ui_control_frame_create_by_def(this->control(), plugin_ui_control_frame_layer_tail, plugin_ui_control_frame_usage_normal, &data.data.swiper.curr_page_frame);

        if (mGrayPageFrame) {
            plugin_ui_control_frame_free(mGrayPageFrame);
        }
        mGrayPageFrame = plugin_ui_control_frame_create_by_def(this->control(), plugin_ui_control_frame_layer_tail, plugin_ui_control_frame_usage_normal, &data.data.swiper.gray_page_frame);
    }
}

/*
virtual
*/
void RGUISwiper::PerformLayout(ui_vector_2_t client_sz) {
    ui_rect pd = GetClientRealPD();
	ui_rect rt = GetRenderTextRT();
	int32_t  w  = ui_rect_width(&rt);
	int32_t  h  = ui_rect_height(&rt);

    client_sz->x = client_sz->y = 0.0f;

	RVector2 pt = RVector2((float)(pd.lt.x - mCurrPage * w), (float)pd.lt.y);
	RVector2 sz = RVector2((float)w,						 (float)h);

    plugin_ui_control_it child_it;
    plugin_ui_control_childs(control(), &child_it);

    for(plugin_ui_control_t child = plugin_ui_control_it_next(&child_it);
        child;
        child = plugin_ui_control_it_next(&child_it))
    {
        RGUIControl * c = (RGUIControl*)plugin_ui_control_product(child);
		if (!c->WasVisible())
			continue;

		c->SetRenderRealPT(pt);
		c->SetRenderRealSZ(sz);

		pt.x += (float)w;
	}
}

void	RGUISwiper::UpdateSelf			( float deltaTime )
{
	RGUIControl::UpdateSelf(deltaTime);

	//更新时间
	if (HasCatchOrChildCatch()) {
		mStayTime += deltaTime;
		mDragTime += deltaTime;
	}
}

void	RGUISwiper::RenderTail			( ui_runtime_render_t ctx, ui_rect_t rect )
{
	// if (mCurrPageFrame) {
	// 	ui_rect rt = GetRenderTextRT();
	// 	int32_t  c  = GetCurrPageCount();
    //     ui_rect_t bounding = plugin_ui_control_frame_bounding_rect(mCurrPageFrame);
	// 	int32_t  w  = ui_rect_width(bounding);
	// 	int32_t  h  = ui_rect_height(bounding);
	// 	int32_t  x  = rt.lt.x + (ui_rect_width(&rt) - w * c) / 2;
	// 	int32_t  y  = rt.lt.y + (ui_rect_height(&rt) - h);

	// 	for (int32_t i = 0; i < c; ++i) {
    //         ui_rect rt = UI_RECT_INITLIZER(x+w*i, y, x+w*i+w, y+h);
    //         ui_rect_adj_by_pt(&rt, plugin_ui_control_real_pt_abs(this->control()));

    //         plugin_ui_control_do_render_frame(
    //             i == mCurrPage ? mCurrPageFrame : mGrayPageFrame,
    //             &rt, NULL, ctx, rect);
	// 	}
	// }
}

/*
event
*/
void RGUISwiper::ProcessDrag(plugin_ui_touch_track_t track) {
	//拖动事件已有控件处理 
	if (plugin_ui_touch_track_process_control(track) &&
		plugin_ui_touch_track_process_control(track) != control())
		return;

    if(GetCurrPageCount() == 0)
        return;

	//动画播放中 
    plugin_ui_control_it child_it;
    plugin_ui_control_childs(control(), &child_it);

    for(plugin_ui_control_t child = plugin_ui_control_it_next(&child_it);
        child;
        child = plugin_ui_control_it_next(&child_it))
    {
        //RGUIControl * c = (RGUIControl*)plugin_ui_control_product(child);

        /*
		if (c->GetMoveAnimCtrl()->WasPlay())
			return;
         */
    }

	if (plugin_ui_touch_track_is_horz_move(track))
	{
		RGUIControl* page = NULL;

        updateCache();

		uint32_t i = 0;

        plugin_ui_control_it child_it;
        plugin_ui_control_childs(control(), &child_it);

        for(plugin_ui_control_t child = plugin_ui_control_it_next(&child_it);
            child;
            child = plugin_ui_control_it_next(&child_it))
        {
            RGUIControl * c = (RGUIControl*)plugin_ui_control_product(child);
        
			if (!c->WasVisible())
				continue;

			if ((uint32_t)mCurrPage == i)
			{
				page = c;
				break;
			}
		}

        if (page == NULL) return;
        
        //拖动
        ui_rect pd = GetClientRealPD();
        float  x  = plugin_ui_touch_track_cur_pt(track)->x - plugin_ui_touch_track_last_pt(track)->x;
		if (x >  0.0f && mCurrPage == 0)
		{
			if (page->GetRenderRealX() > (float)pd.lt.x)
                //todo
				x = 0.0f;
		}
		if (x <  0.0f && mCurrPage == GetCurrPageCount()-1)
		{
			if (page->GetRenderRealX() < (float)pd.lt.x)
                //todo
				x = 0.0f;
		}

		x = ui_pixel_aligned(x);

        plugin_ui_control_childs(control(), &child_it);

        for(plugin_ui_control_t child = plugin_ui_control_it_next(&child_it);
            child;
            child = plugin_ui_control_it_next(&child_it))
        {
            RGUIControl * c = (RGUIControl*)plugin_ui_control_product(child);
        
			c->SetRenderRealX(c->GetRenderRealX() + x);
        }

		//滚动事件处理
        plugin_ui_touch_track_set_process_control(track, control());

		//停滞重置 
		if (mStayTime > 0.05f)
		{
			mDragTime = 0.0f;
			mDragDist = 0.0f;
		}

		//计算拖拽 
		mStayTime  = 0.0f;
		mDragDist += x;
	}
}

void RGUISwiper::ProcessRise(plugin_ui_touch_track_t track) {
    if(!mAcceptRise)  return;

	//拖动事件已有控件处理 
	if (plugin_ui_touch_track_process_control(track) != control())
		return;

    if(GetCurrPageCount() == 0)
        return;

	//动画播放中 
    plugin_ui_control_it child_it;
    plugin_ui_control_childs(control(), &child_it);

    for(plugin_ui_control_t child = plugin_ui_control_it_next(&child_it);
        child;
        child = plugin_ui_control_it_next(&child_it))
    {
        //RGUIControl * c = (RGUIControl*)plugin_ui_control_product(child);
        /*
		if (c->GetMoveAnimCtrl()->WasPlay())
			return;
         */
    }

    if (plugin_ui_touch_track_is_horz_move(track))
	{
		//拖动 
		int32_t c  = GetCurrPageCount();
        float x  = plugin_ui_touch_track_cur_pt(track)->x - plugin_ui_touch_track_down_pt(track)->x;
		int32_t i  = mCurrPage;
		float d1 = GetRenderRealW() * -0.5f;
		float d2 = GetRenderRealW() *  0.5f;
		if (x < d1 || (mDragTime > 0.0f && mStayTime < 0.05f && mDragDist / mDragTime < d1)) 
			i = cpe_limit_in_range(i+1, (int32_t)0, c-1);
		if (x > d2 || (mDragTime > 0.0f && mStayTime < 0.05f && mDragDist / mDragTime > d2)) 
			i = cpe_limit_in_range(i-1, (int32_t)0, c-1);

		SetCurrPage(i);

		//清除 
		mDragTime = 0.0f;
		mStayTime = 0.0f;
		mDragDist = 0.0f;
	}
}

void RGUISwiper::on_show(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    //plugin_ui_control_t self = plugin_ui_control_t(ctx);
}

void RGUISwiper::on_hide(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    //plugin_ui_control_t self = plugin_ui_control_t(ctx);
}

void RGUISwiper::on_layout(plugin_ui_control_t control, ui_vector_2_t client_sz) {
    RGUISwiper * c = cast<RGUISwiper>(control);
    c->PerformLayout(client_sz);
}

void RGUISwiper::on_mouse_rise(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    RGUIControl::on_mouse_rise(ctx, from_control, event);

    RGUISwiper * c = cast<RGUISwiper>((plugin_ui_control_t)ctx);
    c->ProcessRise(plugin_ui_control_touch_track(from_control));
}

void RGUISwiper::on_mouse_drag(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
     RGUISwiper * c = cast<RGUISwiper>((plugin_ui_control_t)ctx);
    if(c->WasAcceptMove()) {
        c->ProcessDrag(plugin_ui_control_touch_track(from_control));
    }
}

void RGUISwiper::setup(plugin_ui_control_meta_t meta) {
    RGUIControl::setup(meta);

    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_hide, plugin_ui_event_scope_childs, on_hide);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_show, plugin_ui_event_scope_childs, on_show);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_up, plugin_ui_event_scope_childs, on_mouse_rise);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_move, plugin_ui_event_scope_childs, on_mouse_drag);
    plugin_ui_control_meta_set_layout(meta, on_layout);
}
