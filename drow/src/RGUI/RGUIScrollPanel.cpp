#include "plugin/ui/plugin_ui_control_meta.h"
#include "RGUIScrollPanel.h"

/*
constructor
*/
RGUIScrollPanel::RGUIScrollPanel()
{
//	mHScrollActivate = false;
}

void RGUIScrollPanel::Load( ui_data_control_t control ) {
    RGUIScrollable::Load(control);

    if (type() == ui_control_type_scroll_panel) {
        UI_CONTROL const & data = *ui_data_control_data(control);
        RGUIScrollable::Load(data.data.scroll_panel.scroll);
    }
}

void RGUIScrollPanel::setup(plugin_ui_control_meta_t meta) {
    RGUIScrollable::setup(meta);
    //plugin_ui_control_meta_set_layout(meta, NULL);
}

void RGUIScrollPanel::PerformLayout(ui_vector_2_t client_sz) {
    plugin_ui_control_basic_layout(control(), client_sz);

    client_sz->x = client_sz->y = 0.0f;

    plugin_ui_control_it child_it;
    plugin_ui_control_childs(control(), &child_it);

    for(plugin_ui_control_t child = plugin_ui_control_it_next(&child_it);
        child;
        child = plugin_ui_control_it_next(&child_it))
    {
        RGUIControl * c = (RGUIControl*)plugin_ui_control_product(child);
        
		float w = c->GetRenderRealX() + c->GetRenderRealW();
		float h = c->GetRenderRealY() + c->GetRenderRealH();
		if (w > client_sz->x) client_sz->x = w;
		if (h > client_sz->y) client_sz->y = h;
	}

	//计算实际客户区大小 
    ui_rect    pd = GetClientRealPD();
    client_sz->x += (pd.lt.x + pd.rb.x);
    client_sz->y += (pd.lt.y + pd.rb.y);
}

