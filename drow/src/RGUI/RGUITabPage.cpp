#include "cpe/pal/pal_stdlib.h"
#include "RGUIToggle.h"
#include "RGUIWindow.h"
#include "RGUITab.h"
#include "RGUITabPage.h"

/*
constructor
*/
RGUITabPage::RGUITabPage()
{
	mToggleText = 			"";
	mToggleTextKey = 		0;
	mToggleTemplateName = 	"";

	mToggleTemplate = NULL;
}

/*
method
*/

void	RGUITabPage::SetToggleTemplateName	( const std::string& name )
{
	if (mToggleTemplateName != name)
	{
		mToggleTemplateName	 = name;
		mToggleTemplate		 = GetTemplate(mToggleTemplateName.c_str());
	}
}

/*
load & save
*/
void RGUITabPage::Load( ui_data_control_t control ) {
    RGUIPanel::Load(control);

    if (type() == ui_control_type_tab_page) {
        UI_CONTROL const & data = *ui_data_control_data(control);
        mToggleText = ui_data_control_msg(control, data.data.tab_page.toggle_text_id);
        mToggleTextKey = data.data.tab_page.toggle_text_key;
        mToggleTemplateName = ui_data_control_msg(control, data.data.tab_page.toggle_template_id);
    }
}

/*
virtual
*/
void RGUITabPage::Retext(void) {
    if (mToggleTextKey != 0) {
        mToggleText = visibleMsg(mToggleTextKey);
    }
}

/*
call back
*/
void RGUITabPage::OnLoadProperty() {
	RGUIPanel::OnLoadProperty();
    
	//得到模版指针 
	mToggleTemplate = GetTemplate(mToggleTemplateName.c_str());
}

//TODO: Loki
// void	RGUITabPage::OnShow			( RGUIEventArgs& args )
// {
// 	RGUIPanel::OnShow(args);

//     RGUIControl * parent = GetParent();
// 	RGUITab* tab = dynamic_cast<RGUITab*>(parent);
// 	if (tab)
// 	{
// 		//隐藏其它页面
//         plugin_ui_control_it child_it;
//         plugin_ui_control_childs(tab->control(), &child_it);

//         for(plugin_ui_control_t child = plugin_ui_control_it_next(&child_it);
//             child;
//             child = plugin_ui_control_it_next(&child_it))
//         {
//             RGUIControl * c = (RGUIControl*)plugin_ui_control_product(child);

// 			if (c == this)
// 				continue;

// 			c->Hide();
// 		}

// 	}
// }
