#include "gd/app/app_log.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_control_meta.h"
#include "plugin/ui/plugin_ui_control_frame.h"
#include "RGUIWindow.h"
#include "RGUIListBoxBase.h"

RGUIListBoxBase::RGUIListBoxBase()
    : mItemTempID(0)
    , mHeadTempID(0)
    , mTailTempID(0)
    , mItemTemplate(NULL)
    , mHeadTemplate(NULL)
    , mTailTemplate(NULL)
{
	mHorzGrap = 			0;
	mVertGrap = 			0;
	mLightFrame = 			NULL;
	mLightFrameShow = 		true;
	mItemFreeSize = 		false;

	mItemCreator  = NULL;
	mHeadCtrl	  = NULL;
	mTailCtrl     = NULL;
	mHeadShow	  = false;
	mTailShow	  = false;
}

RGUIListBoxBase::~RGUIListBoxBase(void) {
    if (mLightFrame) {
        plugin_ui_control_frame_free(mLightFrame);
        mLightFrame = NULL;
    }

	mItemPool.clear();
	mItemList.clear();
}

/*
method
*/
void	RGUIListBoxBase::SetSelectedItem		( uint32_t index, bool flag )
{
	if (mItemList[index].selected == flag)
		return;

	if (flag)
	{
		//把其他子项置回非选中 
		for (uint32_t i = 0; i < mItemList.size(); ++i)
			mItemList[i].selected = false;
	}

	//设置 
	mItemList[index].selected = true;

    plugin_ui_control_dispatch_event(
        control(), control(),
        plugin_ui_event_list_selection_changed, plugin_ui_event_dispatch_to_self_and_parent);
}

/*
item
*/
RGUIListBoxAdvItem*
RGUIListBoxBase::GetItem(RGUIControl* ctrl) {
	RGUIControl*  temp = ctrl;
	while (temp && temp->GetParent() != this)
		temp = temp->GetParent();

	return dynamic_cast<RGUIListBoxAdvItem*>(temp);
}

RGUIListBoxAdvItem*
RGUIListBoxBase::GetItem(uint32_t index) {
	if (index != (uint32_t)-1) {
		//查找 
		for (uint32_t i = 0; i < mItemPool.size(); ++i) {
			if (mItemPool[i]->mDirty) continue;

			if (mItemPool[i]->mIndex == index) {
				return mItemPool[i];
            }
		}
	}

	return NULL;
}

void RGUIListBoxBase::AddItem				( uint32_t index )
{
	AddItem(index, 1);
}

void RGUIListBoxBase::AddItem( uint32_t index, uint32_t count ) {
	ItemInfo info;
	info.pixel		= 0;
	info.selected	= false;

	//添加数据 
	mItemList.insert(
		mItemList.begin()+index, 
		count, 
		info);

	//更改索引 
	for (uint32_t i = 0; i < mItemPool.size(); ++i) {
		if (mItemPool[i]->mDirty)
			continue;

		if (mItemPool[i]->mIndex >= index)
			mItemPool[i]->mIndex += count;
	}

    plugin_ui_control_set_cache_flag_layout(control(), 1);
}

void RGUIListBoxBase::DelItem(uint32_t index) {
	DelItem(index, 1);
}

void RGUIListBoxBase::DelItem(uint32_t index, uint32_t count) {
	//移除数据 
	mItemList.erase(
		mItemList.begin()+index,
		mItemList.begin()+index+count);

	//更改索引 
	for (uint32_t i = 0; i < mItemPool.size(); ++i) {
		if (mItemPool[i]->mDirty) continue;

		if (mItemPool[i]->mIndex >= index && mItemPool[i]->mIndex <  index+count) {
			mItemPool[i]->mDirty  = true;
        }
        
		if (mItemPool[i]->mIndex >= index+count) {
			mItemPool[i]->mIndex -= count;
        }
	}

    plugin_ui_control_set_cache_flag_layout(control(), 1);
}

void RGUIListBoxBase::DelItemAll(void) {
	//移除子项
    plugin_ui_control_it child_it;
    plugin_ui_control_childs(control(), &child_it);

    plugin_ui_control_t next_child = NULL;
    for(plugin_ui_control_t child = plugin_ui_control_it_next(&child_it); child; child = next_child) {
        next_child = plugin_ui_control_it_next(&child_it);
        RGUIListBoxAdvItem * c = RGUIControl::cast<RGUIListBoxAdvItem>(child);
        if (c == NULL || c == mHeadCtrl || c == mTailCtrl) continue;
		plugin_ui_control_free(child);
	}

	mItemPool.clear();
	mItemList.clear();

	//更新滚动 
    plugin_ui_control_set_cache_flag_layout(control(), 1);
}

/*
item pixel
*/
int32_t	RGUIListBoxBase::GetItemPixel			( uint32_t index ) const
{
	return mItemList[index].pixel;
}

void	RGUIListBoxBase::SetItemPixel			( uint32_t index, int32_t pixel )
{
	if (mItemList[index].pixel != pixel) {
		mItemList[index].pixel  = pixel;
        plugin_ui_control_set_cache_flag_layout(control(), 1);
	}
}

/*
load & save & clone
*/
void RGUIListBoxBase::Load( UI_CONTROL_BOX const & data ) {
    mHorzGrap = data.horz_grap;
    mVertGrap = data.vert_grap;
    mLightFrameShow = data.light_frame_show;
    mItemFreeSize = data.item_free_size;
    mItemTempID = data.item_template_id;
    mHeadTempID = data.head_template_id;
    mTailTempID = data.tail_template_id;

    if (mLightFrame) {
        plugin_ui_control_frame_free(mLightFrame);
    }
    
    if (data.light_frame.res.type != UI_OBJECT_TYPE_NONE) {
        mLightFrame =
            plugin_ui_control_frame_create_by_def(
                control(), plugin_ui_control_frame_layer_back, plugin_ui_control_frame_usage_down, &data.light_frame);
    }
    else {
        mLightFrame = NULL;
    }
}

/*
call back
*/
void RGUIListBoxBase::OnLoadProperty() {
	RGUIScrollable::OnLoadProperty();
    
	//得到模板指针 
	mItemTemplate = GetTemplate(mItemTempID);
	mHeadTemplate = GetTemplate(mHeadTempID);
	mTailTemplate = GetTemplate(mTailTempID);

    assert(mHeadCtrl == NULL);
	if (mHeadTemplate) {
        mHeadCtrl = create<RGUIListBoxAdvItem>(mHeadTemplate);
		AddChild(mHeadCtrl);
	}

    assert(mTailCtrl == NULL);
	if (mTailTemplate) {
		mTailCtrl = create<RGUIListBoxAdvItem>(mTailTemplate);
		AddChild(mTailCtrl);
	}
}

/*
event
*/
void RGUIListBoxBase::on_mouse_click(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    RGUIListBoxBase * c = cast<RGUIListBoxBase>((plugin_ui_control_t)ctx);
    assert(c);

    if (c->mTopBtn && c->mTopBtn->control() == from_control) {
        c->onLineUp();
    }
    else if (c->mBottomBtn && c->mBottomBtn->control() == from_control) {
        c->onLineDown();
    }
    else if (c->mLeftBtn && c->mLeftBtn->control() == from_control) {
        c->onLineLeft();
    }
    else if (c->mRightBtn && c->mRightBtn->control() == from_control) {
        c->onLineRight();
    }
    else {
        if (!plugin_ui_control_is_child_of_r(from_control, c->control())) return;

        RGUIListBoxAdvItem* item = c->GetItem(cast(from_control));
        if (item == NULL	  ||
            item == c->mHeadCtrl ||
            item == c->mTailCtrl ||
            item->mDirty)
            return;

        uint32_t index = item->mIndex;
        c->SetSelectedItem(index, !c->mItemList[index].selected);
    }
}

void RGUIListBoxBase::on_vscroll_changed(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    RGUIScrollable::on_vscroll_changed(ctx, from_control, event);
	if (from_control == ctx) {
        plugin_ui_control_t control = (plugin_ui_control_t)ctx;
        plugin_ui_control_set_cache_flag_layout(control, 1);
	}
}

void RGUIListBoxBase::on_hscroll_changed(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    RGUIScrollable::on_hscroll_changed(ctx, from_control, event);
	if (from_control == ctx) {
        plugin_ui_control_t control = (plugin_ui_control_t)ctx;
        plugin_ui_control_set_cache_flag_layout(control, 1);
	}
}

//void RGUIListBoxBase::RenderTail(ui_runtime_render_t ctx, ui_rect_t rect) {
	// RGUIScrollable::RenderTail(ctx, rect);

	// if (mLightFrameShow && mLightFrame) {
    //     for (uint32_t i = 0; i < mItemPool.size(); ++i) {
    //         RGUIListBoxAdvItem* item = mItemPool[i];
    //         if (item->mDirty) continue;

    //         if (mItemList[item->mIndex].selected || item->HasCatchOrChildCatch()) {
    //             ui_rect rt = plugin_ui_control_real_rt_no_scale(item->control());
    //             plugin_ui_control_do_render_frame(mLightFrame, &rt, NULL, ctx, rect);
    //         }
    //     }
	// }
//}

void RGUIListBoxBase::ItemTackback( void ) {
	uint32_t start = GetViewStart();
	uint32_t final = GetViewFinal();

	for (uint32_t i = 0; i < mItemPool.size(); ++i) {
		if (mItemPool[i]->mDirty) continue;

		if (mItemPool[i]->mIndex < start || mItemPool[i]->mIndex > final) {
			mItemPool[i]->mDirty = true;
        }
	}
}

void RGUIListBoxBase::UpdateItemAll( void ) {
	uint32_t start = GetViewStart();
	uint32_t final = GetViewFinal();

	for (uint32_t i = 0; i < mItemPool.size(); ++i) {
		if (mItemPool[i]->mDirty) continue;

		if (mItemPool[i]->mIndex < start || mItemPool[i]->mIndex > final) {
			mItemPool[i]->mDirty = true;
            continue;
        }

        plugin_ui_control_dispatch_event(
            control(), mItemPool[i]->control(),
            plugin_ui_event_list_item_show, plugin_ui_event_dispatch_to_self);
	}
}

void RGUIListBoxBase::SetItemAllVisible(bool visible) {
	for (uint32_t i = 0; i < mItemPool.size(); ++i) {
        plugin_ui_control_it child_it;
        plugin_ui_control_childs(mItemPool[i]->control(), &child_it);

        for(plugin_ui_control_t child = plugin_ui_control_it_next(&child_it);
            child;
            child = plugin_ui_control_it_next(&child_it))
        {
            RGUIControl * c = (RGUIControl*)plugin_ui_control_product(child);

			c->SetVisible(visible);
		}
	}
}

void RGUIListBoxBase::ItemAllocate(void) {
	if (mItemList.empty()) return;

	uint32_t start = GetViewStart();
	uint32_t final = GetViewFinal();
	uint32_t count = final - start + 1;

	if (count > 0) {
		std::vector<bool> existVec;
		existVec.resize(count, false);
		for (uint32_t i = 0; i < mItemPool.size(); ++i) {
			RGUIListBoxAdvItem* item = mItemPool[i];
			if (item->mDirty)
				continue;

			existVec[item->mIndex-start] = true;
		}

		for (uint32_t i = 0; i < count; ++i) {
			if (existVec[i]) continue;

			ItemAllocate(start+i);
		}
	}
}

void RGUIListBoxBase::ItemAllocate(uint32_t index) {
	if (mItemTemplate == NULL) return;
    
    RGUIListBoxAdvItem* item = NULL;

    //查找 
    for (uint32_t i = 0; i < mItemPool.size(); ++i) {
        if (mItemPool[i]->mDirty) {
            item = mItemPool[i];
            break;
        }
    }

    //创建 
    if (item == NULL) {
        item = mItemCreator ? mItemCreator() : create<RGUIListBoxAdvItem>();
        item->SetTemplateLinkCtrl(mItemTemplate);
        AddChild(item);
        mItemPool.push_back(item);
    }

    //添加 
    item->mIndex = index;
    item->mDirty = false;

    plugin_ui_control_dispatch_event(
        control(), item->control(),
        plugin_ui_event_list_item_show, plugin_ui_event_dispatch_to_self);
}

void RGUIListBoxBase::on_layout(plugin_ui_control_t control, ui_vector_2_t client_sz) {
    RGUIListBoxBase * p = (RGUIListBoxBase*)plugin_ui_control_product(control);
	p->ItemTackback();
	p->ItemAllocate();
    plugin_ui_control_set_cache_flag_layout(control, 0);
    
    RGUIScrollable::on_layout(control, client_sz);
}

void RGUIListBoxBase::onLineUp(void) {
}

void RGUIListBoxBase::onLineDown(void) {
}

void RGUIListBoxBase::onLineLeft(void) {
}

void RGUIListBoxBase::onLineRight(void) {
}

void RGUIListBoxBase::setup(plugin_ui_control_meta_t meta) {
    RGUIScrollable::setup(meta);

    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_click, plugin_ui_event_scope_childs, on_mouse_click);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_vscroll_changed, plugin_ui_event_scope_self, on_vscroll_changed);
    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_hscroll_changed, plugin_ui_event_scope_self, on_hscroll_changed);
    plugin_ui_control_meta_set_layout(meta, on_layout);    
}

