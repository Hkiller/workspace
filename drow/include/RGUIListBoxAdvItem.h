#ifndef DROW_UI_CONTROL_LISTBOXADVITEM_H
#define DROW_UI_CONTROL_LISTBOXADVITEM_H
#include "RGUIPanel.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUIListBoxAdvItem, ui_control_type_list_box_adv_item);

class RGUIListBoxAdvItem : public RGUIPanel {
public:
	RGUIListBoxAdvItem();

	uint32_t GetIndex(void) const { return mIndex; }

protected:
	uint32_t mIndex;
	bool mDirty;

	virtual ~RGUIListBoxAdvItem(void);
    
	friend class RGUIListBoxBase;
	friend class RGUIListBoxRow;
	friend class RGUIListBoxCol;
    friend class RGUIControlRepo; 
};

#endif
