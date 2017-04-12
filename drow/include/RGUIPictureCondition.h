#ifndef DROW_UI_CONTROL_PICTURECONDITION_H
#define DROW_UI_CONTROL_PICTURECONDITION_H
#include "RGUIPicture.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUIPictureCondition, ui_control_type_picture_cond);

class RGUIPictureCondition : public RGUIPicture {
public:
	RGUIPictureCondition();

	uint32_t GetIndex(void) const { return mIndex; }
	virtual void SetIndex(uint32_t index);
	void DelRenderDataAll(void);

	/*
	load & save & clone
	*/
    virtual void        Load      ( ui_data_control_t control );
    static  int         index_setter(plugin_ui_control_t control, dr_value_t value);
    static void         setup(plugin_ui_control_meta_t meta);

protected:
    void updateFrames(void);
    
	uint32_t mIndex;

    ~RGUIPictureCondition();
    friend class RGUIControlRepo; 
};

#endif
