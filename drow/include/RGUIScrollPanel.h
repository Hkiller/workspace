#ifndef DROW_UI_CONTROL_SCROLLPANEL_H
#define DROW_UI_CONTROL_SCROLLPANEL_H
#include "RGUIScrollable.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUIScrollPanel, ui_control_type_scroll_panel);

class RGUIScrollPanel : public RGUIScrollable {
public:
	RGUIScrollPanel();

    virtual void Load(ui_data_control_t control);
    virtual void PerformLayout(ui_vector_2_t client_sz);

    static void setup(plugin_ui_control_meta_t meta);

protected:
	virtual ~RGUIScrollPanel( void ){};
    friend class RGUIControlRepo; 
};


#endif
