#include "cpe/utils/math_ex.h"
#include "plugin/ui/plugin_ui_control_meta.h"
#include "plugin/ui/plugin_ui_env.h"
#include "RGUIWindow.h"
#include "RGUIListBoxCol.h"

RGUIListBoxCol::RGUIListBoxCol()
    : mColCount(1)
{
	mHScrollActivate = false;
}

uint32_t RGUIListBoxCol::GetColCount(void) const {
	return mColCount;
}

void RGUIListBoxCol::SetColCount(uint32_t count) {
	if (mColCount != count) {
		mColCount  = count;
        plugin_ui_control_set_cache_flag_layout(control(), 1);
	}
}

void RGUIListBoxCol::SetListBoxScrollIndex(uint32_t index){
    if (mItemTemplate == NULL) return;

    RGUIUnitVec2 unit = RGUIUnitVec2::ToUnit(
        ui_data_control_data(mItemTemplate)->basic.accept_sz_ls,
        GetEditorRealSZ(),
        GetEditorRealSZ(),
        ui_data_control_data(mItemTemplate)->basic.editor_sz);
    RVector2     size = unit.ToReal(
        ui_data_control_data(mItemTemplate)->basic.accept_sz_ls, GetRenderRealSZ(), GetEditorRealSZ());

    if (size.h <= 0) return;
    int32_t th = (int32_t)size.h;

    SetScrollRealY(int32_t(th + mVertGrap)*index);
}

uint32_t RGUIListBoxCol::GetItemIndex(int32_t point) const {
    if (mItemTemplate == NULL) return 0;

    ui_rect pd = GetClientRealPD();
    RGUIUnitVec2 unit = RGUIUnitVec2::ToUnit(
        ui_data_control_data(mItemTemplate)->basic.accept_sz_ls,
        GetEditorRealSZ(),
        GetEditorRealSZ(),
        ui_data_control_data(mItemTemplate)->basic.editor_sz);
    RVector2 size = unit.ToReal(
        ui_data_control_data(mItemTemplate)->basic.accept_sz_ls, GetRenderRealSZ(), GetEditorRealSZ());

    if (size.h <= 0) return 0;

    int32_t th = (int32_t)size.h;
    int32_t h  = 0;
    int32_t y  = 0;
    int32_t i  = 0;

    if (mColCount == 1 && mItemFreeSize) {
        y = pd.lt.y - GetScrollRealY();
        for (; i < (int32_t)mItemList.size(); ++i) {
            h = (mItemList[i].pixel == 0 ? th : mItemList[i].pixel) + mVertGrap;
            if (point <= y+h)
                break;

            y += h;
        }
    }
    else {
        y = point - pd.lt.y + GetScrollRealY();
        h = th + mVertGrap;
		if(h > 0) {
			i = y / h * mColCount;
		}
    }

    return (uint32_t)cpe_limit_in_range(i, (int32_t)0, (int32_t)(mItemList.size()-1));
}

int32_t	RGUIListBoxCol::GetItemPixel(void) const {
	if (mItemTemplate == NULL) return 0;

    RGUIUnitVec2 unit = RGUIUnitVec2::ToUnit(
        ui_data_control_data(mItemTemplate)->basic.accept_sz_ls,
        GetEditorRealSZ(),
        GetEditorRealSZ(),
        ui_data_control_data(mItemTemplate)->basic.editor_sz);
    RVector2     size = unit.ToReal(
        ui_data_control_data(mItemTemplate)->basic.accept_sz_ls, GetRenderRealSZ(), GetEditorRealSZ());

    int32_t th = (int32_t)size.h;
    int32_t p  = 0;

    if (mColCount == 1 && mItemFreeSize) {
        for (uint32_t i = 0; i < mItemList.size(); ++i) {
            p += (mItemList[i].pixel == 0 ? th : mItemList[i].pixel) + mVertGrap;
        }
    }
    else {
        p = (th + mVertGrap) * (int32_t)ceil((float)mItemList.size() / (float)mColCount);
    }

    return p;
}

uint32_t RGUIListBoxCol::GetViewStart(void) const {
    if (mItemList.empty()) return 0;

	return cpe_limit_in_range(
		(int32_t)GetItemIndex(GetRenderTextRT().lt.y),
		(int32_t)0,
		((int32_t)mItemList.size())-1);
}

uint32_t RGUIListBoxCol::GetViewFinal( void ) const {
    if (mItemList.empty()) return 0;

	return cpe_limit_in_range(
		(int32_t)GetItemIndex(GetRenderTextRT().rb.y)+(int32_t)mColCount-1,
		(int32_t)0,
		((int32_t)(mItemList.size()))-1);
}

void RGUIListBoxCol::Load( ui_data_control_t control ) {
    RGUIListBoxBase::Load(control);

    if (type() == ui_control_type_list_box_col) {
        UI_CONTROL const & data = *ui_data_control_data(control);
        RGUIScrollable::Load(data.data.list_box_col.scroll);
        RGUIListBoxBase::Load(data.data.list_box_col.box);
        mColCount = data.data.list_box_col.col_count;
    }
}

void RGUIListBoxCol::PerformLayout(ui_vector_2_t client_sz) {
    client_sz->x = client_sz->y = 0.0f;
    
	if (mItemTemplate == NULL) return;

    ui_rect pd = GetClientRealPD();

    RGUIUnitVec2 unit = RGUIUnitVec2::ToUnit(
        ui_data_control_data(mItemTemplate)->basic.accept_sz_ls,
        GetEditorRealSZ(),
        GetEditorRealSZ(),
        ui_data_control_data(mItemTemplate)->basic.editor_sz);
        
    RVector2 size = unit.ToReal(
        ui_data_control_data(mItemTemplate)->basic.accept_sz_ls, GetRenderRealSZ(), GetEditorRealSZ());
    
    float tw = size.w;
    float th = size.h;
    float h  = (float)mVertGrap + th;
    float x  = (float)pd.lt.x;
    float y  = (float)pd.lt.y;

    //头部 
    if (mHeadCtrl) 
        mHeadCtrl->SetRenderRealPT(RVector2(x, y - mHeadCtrl->GetRenderRealH()));
    //尾部 
    if (mTailCtrl)
        mTailCtrl->SetRenderRealPT(RVector2(x, y + (float)GetItemPixel()));

    //子项 
    if (mColCount == 1 && mItemFreeSize) {
        for (uint32_t i = 0; i < mItemList.size(); ++i) {
            h = (mItemList[i].pixel == 0 ? th : (float)mItemList[i].pixel);

            RGUIListBoxAdvItem* item = GetItem(i);
            if (item) {
                item->SetRenderRealPT(RVector2(x , y));
                item->SetRenderRealSZ(RVector2(tw, h));
            }

            y += h;
            y += (float)mVertGrap;
        }
    }
    else {
        for (uint32_t i = 0; i < mItemPool.size(); ++i) {
            if (mItemPool[i]->mDirty) continue;

            /*设置位置 */
            uint32_t index = mItemPool[i]->mIndex;
            float row   = (float)(index / mColCount);
            float col   = (float)(index % mColCount);
            mItemPool[i]->SetRenderRealPT(RVector2(x+col*tw, y+row*h));
            mItemPool[i]->SetRenderRealSZ(size);
        }
    }
}

void RGUIListBoxCol::on_mouse_rise(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    //RGUIListBoxBase::on_mouse_rise(ctx, from_control, event);

    //RGUIListBoxCol * c = cast<RGUIListBoxCol>((plugin_ui_control_t)ctx);

	//滑动判定
    /*
	if (c->mMouseStayTime < 0.1f && c->mMouseDragTime > 0.0f) {
		if (c->mVScrollActivate) {
			int32_t scrollMin = (c->mHeadCtrl && c->mHeadShow) ? (int32_t)(-c->mHeadCtrl->GetRenderRealH())					: 0;
 			int32_t scrollMax = (c->mTailCtrl && c->mTailShow) ? (int32_t)(c->mTailCtrl->GetRenderRealH() + c->mVScrollRange)  : c->mVScrollRange;

			if (c->GetScrollRealY() < scrollMin || c->GetScrollRealY() > scrollMax) {
				if (c->GetScrollRealY() < scrollMin) c->SetVScrollValueAnim(scrollMin, 15, scrollMin, scrollMax);
				if (c->GetScrollRealY() > scrollMax) c->SetVScrollValueAnim(scrollMax, 15, scrollMin, scrollMax);
			}
			else {
				int32_t delta = (int32_t)(c->mMouseDragDist.y / c->mMouseDragTime) / 2;
				int32_t value = c->GetScrollRealY() - delta;
				c->SetVScrollValueAnim(value, 60, scrollMin, scrollMax);
			}
		}
	}
*/

}

void RGUIListBoxCol::on_vscroll_changed(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
	RGUIListBoxBase::on_vscroll_changed(ctx, from_control, event);
    
	if (ctx == from_control) {
        RGUIListBoxCol * c = cast<RGUIListBoxCol>((plugin_ui_control_t)ctx);

		if (c->mHeadCtrl && c->mHeadCtrl->WasVisible() && !c->mHeadShow && c->GetScrollRealY() < (int32_t)(-c->mHeadCtrl->GetRenderRealH())) {
			c->SetHeadShow(true);

            plugin_ui_control_dispatch_event(
                c->control(), c->control(),
                plugin_ui_event_list_head_show, plugin_ui_event_dispatch_to_self_and_parent);
		}

		if (c->mTailCtrl && c->mTailCtrl->WasVisible() && !c->mTailShow && c->GetScrollRealY() > (int32_t)(c->mTailCtrl->GetRenderRealH() + c->mVScrollRange)) {
			c->SetTailShow(true);

            plugin_ui_control_dispatch_event(
                c->control(), c->control(),
                plugin_ui_event_list_tail_show, plugin_ui_event_dispatch_to_self_and_parent);
		}
	}
}

void RGUIListBoxCol::UpdateSelf(float deltaTime) {
	RGUIControl::UpdateSelf(deltaTime);

	if (HasCatchOrChildCatch()) {
		//mMouseStayTime += deltaTime;
		//mMouseDragTime += deltaTime;
	}
	else {
		// if (!mVScrollAnimCtrl->WasPlay()) {
		// 	int32_t scrollMin = (mHeadCtrl && mHeadShow) ? (int32_t)(-mHeadCtrl->GetRenderRealH())					: 0;
		// 	int32_t scrollMax = (mTailCtrl && mTailShow) ? (int32_t)( mTailCtrl->GetRenderRealH() + mVScrollRange)  : mVScrollRange;

		// 	if (GetScrollRealY() < scrollMin || GetScrollRealY() > scrollMax) {
		// 		if (GetScrollRealY() < scrollMin)	SetVScrollValueAnim(scrollMin, 15, scrollMin, scrollMax);
		// 		if (GetScrollRealY() > scrollMax)	SetVScrollValueAnim(scrollMax, 15, scrollMin, scrollMax);
		// 	}
		// 	else {
		// 		if (mVScrollAutoHide && mVScrollAlpha != 0.0f)
		// 			SetVScrollAlphaAnim();
		// 	}
		// }
	}
}

void RGUIListBoxCol::UpdateScrollRange(ui_vector_2_t client_sz) {
    ui_rect rt = GetRenderTextRT();
	mVScrollRange = cpe_max(GetItemPixel() - ui_rect_height(&rt), 0.0f);
}

void RGUIListBoxCol::setup(plugin_ui_control_meta_t meta) {
    RGUIListBoxBase::setup(meta);

    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_vscroll_changed, plugin_ui_event_scope_self, on_vscroll_changed);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_up, plugin_ui_event_scope_all, on_mouse_rise);
}
