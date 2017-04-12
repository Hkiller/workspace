#ifndef  DROW_UI_CONTROL_PICTURE_H
#define  DROW_UI_CONTROL_PICTURE_H
#include "RGUIControl.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUIPicture, ui_control_type_picture);

class RGUIPicture : public RGUIControl {
public:
	RGUIPicture();

protected:
    ~RGUIPicture();
    
    friend class RGUIControlRepo; 
};

#endif
