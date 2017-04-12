#ifndef DROW_UI_CONTROL_RICHTEXT_H
#define DROW_UI_CONTROL_RICHTEXT_H
#include "RGUILabel.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUIRichText, ui_control_type_rich_text);

class RGUIRichText : public RGUILabel {
public:
    RGUIRichText(const char * layout = "rich");

    virtual void Load(ui_data_control_t control);
    
protected:
	virtual ~RGUIRichText();
    friend class RGUIControlRepo; 
};

#endif
