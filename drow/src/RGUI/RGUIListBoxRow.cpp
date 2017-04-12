#include "cpe/utils/math_ex.h"
#include "plugin/ui/plugin_ui_control_meta.h"
#include "plugin/ui/plugin_ui_env.h"
#include "RGUIWindow.h"
#include "RGUIListBoxRow.h"

RGUIListBoxRow::RGUIListBoxRow() 
    : mRowCount(1)
{
	mVScrollActivate = false;
}

RGUIListBoxRow::~RGUIListBoxRow(void) {
}

uint32_t RGUIListBoxRow::GetRowCount(void) const {
	return mRowCount;
}

void RGUIListBoxRow::SetRowCount(uint32_t count) {
	if (mRowCount != count) {
		mRowCount  = count;
        plugin_ui_control_set_cache_flag_layout(control(), 1);
	}
}

uint32_t RGUIListBoxRow::GetItemIndex(int32_t point) const {
	if (mItemTemplate) {
        ui_rect pd = GetClientRealPD();
        RGUIUnitVec2 unit = RGUIUnitVec2::ToUnit(
            ui_data_control_data(mItemTemplate)->basic.accept_sz_ls,
            GetEditorRealSZ(),
            GetEditorRealSZ(),
            ui_data_control_data(mItemTemplate)->basic.editor_sz);
        
        RVector2 size = unit.ToReal(
            ui_data_control_data(mItemTemplate)->basic.accept_sz_ls, GetRenderRealSZ(), GetEditorRealSZ());

		int32_t tw = (int32_t)size.w;
		int32_t w  = 0;
		int32_t x  = 0;
		int32_t i  = 0;

		if (mRowCount == 1 && mItemFreeSize) {
			x = pd.lt.x - GetScrollRealX();
			for (; i < (int32_t)mItemList.size(); ++i) {
				w = (mItemList[i].pixel == 0 ? tw : mItemList[i].pixel) + mHorzGrap;
				if (point <= x+w) break;
				x += w;
			}
		}
		else {
			x = point - pd.lt.x + GetScrollRealX();
			w = tw + mHorzGrap;
			i = x / w * mRowCount;
		}

		return (uint32_t)cpe_limit_in_range(i, (int32_t)0, (int32_t)(mItemList.size()-1));
	}

	return 0;
}

int32_t RGUIListBoxRow::GetItemPixel(void) const {
	if (mItemTemplate == NULL) return 0;

    RGUIUnitVec2 unit = RGUIUnitVec2::ToUnit(
        ui_data_control_data(mItemTemplate)->basic.accept_sz_ls,
        GetEditorRealSZ(),
        GetEditorRealSZ(),
        ui_data_control_data(mItemTemplate)->basic.editor_sz);
        
    RVector2 size = unit.ToReal(
        ui_data_control_data(mItemTemplate)->basic.accept_sz_ls,
        GetRenderRealSZ(),
        GetEditorRealSZ());

    int32_t tw = (int32_t)size.w;
    int32_t p  = 0;

    if (mRowCount == 1 && mItemFreeSize) {
        for (uint32_t i = 0; i < mItemList.size(); ++i) {
            p += (mItemList[i].pixel == 0 ? tw : mItemList[i].pixel) + mHorzGrap;
        }
    }
    else {
        p = (tw + mHorzGrap) * (int32_t)ceil((float)mItemList.size() / (float)mRowCount);
    }

    return p;
}

uint32_t RGUIListBoxRow::GetViewStart(void) const {
    if (mItemList.empty()) return 0;
    
	return cpe_limit_in_range(
		(int32_t)GetItemIndex(GetRenderTextRT().lt.x),				
		(int32_t)0, 
		((int32_t)(mItemList.size())) - 1);
}

uint32_t RGUIListBoxRow::GetViewFinal(void) const {
    if (mItemList.empty()) return 0;
    
	return cpe_limit_in_range(
		(int32_t)GetItemIndex(GetRenderTextRT().rb.x)+mRowCount-1, 
		(int32_t)0, 
		((int32_t)(mItemList.size()))-1u);
}

void RGUIListBoxRow::Load( ui_data_control_t control ) {
    RGUIListBoxBase::Load(control);

    if (type() == ui_control_type_list_box_row) {
        UI_CONTROL const & data = *ui_data_control_data(control);
        RGUIScrollable::Load(data.data.list_box_row.scroll);
        RGUIListBoxBase::Load(data.data.list_box_row.box);
        mRowCount = data.data.list_box_row.row_count;
    }
}

void RGUIListBoxRow::PerformLayout(ui_vector_2_t client_sz) {
    client_sz->x = client_sz->y = 0.0f;

	if (mItemTemplate == NULL) return;

    ui_rect pd = GetClientRealPD();
    RGUIUnitVec2 unit = RGUIUnitVec2::ToUnit(
        ui_data_control_data(mItemTemplate)->basic.accept_sz_ls,
        GetEditorRealSZ(),
        GetEditorRealSZ(),
        ui_data_control_data(mItemTemplate)->basic.editor_sz);

    RVector2 size = unit.ToReal(
        ui_data_control_data(mItemTemplate)->basic.accept_sz_ls,
        GetRenderRealSZ(),
        GetEditorRealSZ());

    float tw = size.w;
    float th = size.h;
    float w  = (float)mHorzGrap + tw;
    float x  = (float)pd.lt.x;
    float y  = (float)pd.lt.y;

    /*头部 */
    if (mHeadCtrl)
        mHeadCtrl->SetRenderRealPT(RVector2(x - mHeadCtrl->GetRenderRealW(), y));
    
    /*尾部 */
    if (mTailCtrl)
        mTailCtrl->SetRenderRealPT(RVector2(x + (float)GetItemPixel(), y));

    /*子项 */
    if (mRowCount == 1 && mItemFreeSize) {
        for (uint32_t i = 0; i < mItemList.size(); ++i) {
            w = (mItemList[i].pixel == 0 ? tw : (float)mItemList[i].pixel);

            RGUIListBoxAdvItem* item = GetItem(i);
            if (item) {
                item->SetRenderRealPT(RVector2(x,  y));
                item->SetRenderRealSZ(RVector2(w, th));
            }

            x += w;
            x += (float)mHorzGrap;
        }
    }
    else {
        for (uint32_t i = 0; i < mItemPool.size(); ++i) {
            if (mItemPool[i]->mDirty) continue;

            uint32_t index = mItemPool[i]->mIndex;
            uint32_t col = index / mRowCount;
            uint32_t row = index % mRowCount;

            mItemPool[i]->SetRenderRealPT(RVector2(x+col*w, y+row*th));
            mItemPool[i]->SetRenderRealSZ(size);            
        }
    }
}

void RGUIListBoxRow::on_mouse_rise(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    //RGUIListBoxRow * c = cast<RGUIListBoxRow>((plugin_ui_control_t)ctx);
    
	//滑动判定
    /*
	if (c->mMouseStayTime < 0.1f && c->mMouseDragTime > 0.0f) {
		if (c->mHScrollActivate) {
			int32_t scrollMin = (c->mHeadCtrl && c->mHeadShow) ? (int32_t)(-c->mHeadCtrl->GetRenderRealW())					: 0;
			int32_t scrollMax = (c->mTailCtrl && c->mTailShow) ? (int32_t)(c->mTailCtrl->GetRenderRealW() + c->mHScrollRange)  : c->mHScrollRange;

			if (c->GetScrollRealX() < scrollMin || c->GetScrollRealX() > scrollMax) {
				if (c->GetScrollRealX() < scrollMin) c->SetHScrollValueAnim(scrollMin, 15, scrollMin, scrollMax);
				if (c->GetScrollRealX() > scrollMax) c->SetHScrollValueAnim(scrollMax, 15, scrollMin, scrollMax);
			}
			else {
				int32_t delta = (int32_t)(c->mMouseDragDist.x / c->mMouseDragTime) / 2;
				int32_t value = c->GetScrollRealX() - delta;
				c->SetHScrollValueAnim(value, 60, scrollMin, scrollMax);
			}
		}
	}
     */
    
	//清除 
	//c->mMouseDragTime = 0.0f;
	//c->mMouseStayTime = 0.0f;
	//c->mMouseDragDist = RVector2::Zero;
}

void RGUIListBoxRow::on_hscroll_changed(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
	RGUIListBoxBase::on_hscroll_changed(ctx, from_control, event);
    
	if (ctx == from_control) {
        RGUIListBoxRow * c = cast<RGUIListBoxRow>((plugin_ui_control_t)ctx);
        
		if (c->mHeadCtrl && c->mHeadCtrl->WasVisible() && !c->mHeadShow && c->GetScrollRealX() < (int32_t)(-c->mHeadCtrl->GetRenderRealW()))
		{
			c->SetHeadShow(true);
            plugin_ui_control_dispatch_event(
                c->control(), c->control(),
                plugin_ui_event_list_head_show, plugin_ui_event_dispatch_to_self_and_parent);
		}

		if (c->mTailCtrl && c->mTailCtrl->WasVisible() && !c->mTailShow && c->GetScrollRealX() > (int32_t)(c->mTailCtrl->GetRenderRealW() + c->mHScrollRange))
		{
			c->SetTailShow(true);
            plugin_ui_control_dispatch_event(
                c->control(), c->control(),
                plugin_ui_event_list_tail_show, plugin_ui_event_dispatch_to_self_and_parent);
		}
	}
}

/*
method
*/
void RGUIListBoxRow::UpdateSelf(float deltaTime) {
	RGUIControl::UpdateSelf(deltaTime);

	//更新时间
	if (HasCatchOrChildCatch()) {
	}
	else {
		//计算 
		// if (!mHScrollAnimCtrl->WasPlay())
		// {
		// 	int32_t scrollMin = (mHeadCtrl && mHeadShow) ? (int32_t)(-mHeadCtrl->GetRenderRealW())					: 0;
		// 	int32_t scrollMax = (mTailCtrl && mTailShow) ? (int32_t)( mTailCtrl->GetRenderRealW() + mHScrollRange)  : mHScrollRange;

		// 	if (GetScrollRealX() < scrollMin || 
		// 		GetScrollRealX() > scrollMax)
		// 	{
		// 		if (GetScrollRealX() < scrollMin)	SetHScrollValueAnim(scrollMin, 15, scrollMin, scrollMax);
		// 		if (GetScrollRealX() > scrollMax)	SetHScrollValueAnim(scrollMax, 15, scrollMin, scrollMax);
		// 	}
		// 	else
		// 	{
		// 		if (mHScrollAutoHide && mHScrollAlpha != 0.0f)
		// 			SetHScrollAlphaAnim();
		// 	}
		// }
	}
}

void RGUIListBoxRow::UpdateScrollRange(ui_vector_2_t client_sz) {
    ui_rect rt = GetRenderTextRT();
	mHScrollRange = cpe_max(GetItemPixel() - (int32_t)ui_rect_width(&rt), (int32_t)0);
	mVScrollRange = 0;
}

void RGUIListBoxRow::setup(plugin_ui_control_meta_t meta) {
    RGUIListBoxBase::setup(meta);
    
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_hscroll_changed, plugin_ui_event_scope_self, on_hscroll_changed);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_up, plugin_ui_event_scope_all, on_mouse_rise);
}
