#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_control_meta.h"
#include "RGUIWindow.h"
#include "RGUIComboBoxDropList.h"

/*
constructor
*/
RGUIComboBoxDropList::RGUIComboBoxDropList()
{
	mMaxDropItem = 	8;
	//列表不接受剪裁 
    SetAcceptClip(false);
}

/*
load & save & clone
*/
void RGUIComboBoxDropList::Load( ui_data_control_t control ) {
    RGUIListBoxCol::Load(control);
    
    if (type() == ui_control_type_comb_box_drop_list) {
        UI_CONTROL const & data = *ui_data_control_data(control);
        RGUIScrollable::Load(data.data.comb_box_drop_list.scroll);
        RGUIListBoxBase::Load(data.data.comb_box_drop_list.box);
        
        //return;
        
        //TODO: Loki
        UI_CONTROL const & ins_data = *ui_data_control_data(control);
        //ui_data_control_t ins_control = plugin_ui_control_src(control);
        //UI_CONTROL const & ins_data = *ui_data_control_data(ins_control);
        mColCount = ins_data.data.comb_box_drop_list.col_count;
        mMaxDropItem = ins_data.data.comb_box_drop_list.max_drop_item;
    }
}

/*
call back
*/
void RGUIComboBoxDropList::OnLoadProperty() {
	RGUIListBoxCol::OnLoadProperty();
	//Hide();
}

/*
event
*/
void RGUIComboBoxDropList::on_mouse_rise(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
	RGUIListBoxCol::on_mouse_rise(ctx, from_control, event);

    RGUIComboBoxDropList * c = cast<RGUIComboBoxDropList>((plugin_ui_control_t)ctx);
    
	//失去焦点
    plugin_ui_env_set_focus_control(c->ui_env(), NULL);
}

/*
method
*/
void	RGUIComboBoxDropList::RenderTail			( ui_runtime_render_t ctx, ui_rect_t rect )
{
	//RGUIScrollable::RenderTail(ctx, rect);

	// if (mLightFrameShow && mLightFrame) {
	// 	{
    //         plugin_ui_control_it child_it;
    //         plugin_ui_control_childs(control(), &child_it);

    //         for(plugin_ui_control_t child = plugin_ui_control_it_next(&child_it);
    //             child;
    //             child = plugin_ui_control_it_next(&child_it))
    //         {
    //             RGUIControl * c = (RGUIControl*)plugin_ui_control_product(child);
            
	// 			RGUIListBoxAdvItem* item = dynamic_cast<RGUIListBoxAdvItem *>(c);
	// 			if (item->HasCatchOrChildCatch()) {
    //                 ui_rect rt = plugin_ui_control_real_rt_no_scale(item->control());
    //                 plugin_ui_control_do_render_frame(mLightFrame, &rt, NULL, ctx, rect);
	// 			}
	// 		}
	// 	}
	// }
}

void RGUIComboBoxDropList::setup(plugin_ui_control_meta_t meta) {
    RGUIListBoxCol::setup(meta);

    plugin_ui_control_meta_set_event_fun(meta, plugin_ui_event_mouse_up, plugin_ui_event_scope_childs, on_mouse_rise);    
}
