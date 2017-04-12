#ifndef DROW_UI_CONTROL_PANEL_H
#define DROW_UI_CONTROL_PANEL_H
#include "RGUIControl.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUIPanel, ui_control_type_panel);

class RGUIPanel : public RGUIControl {
public:
	RGUIPanel();

protected:
    ~RGUIPanel() {}

    friend class RGUIControlRepo; 
};

#endif
