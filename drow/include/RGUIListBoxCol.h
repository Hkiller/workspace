#ifndef DROW_UI_CONTROL_LISTBOXCOL_H
#define DROW_UI_CONTROL_LISTBOXCOL_H
#include "RGUIListBoxBase.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUIListBoxCol, ui_control_type_list_box_col);

class RGUIListBoxCol : public RGUIListBoxBase {
public:
	/*
	constructor
	*/
	RGUIListBoxCol();

	/*
	method
	*/
	uint32_t GetColCount(void) const;
	void SetColCount(uint32_t count);

	/*
	item
	*/
	virtual uint32_t				GetItemIndex			( int32_t point ) const;
	virtual int32_t				GetItemPixel			( void )		const;
	virtual uint32_t				GetViewStart			( void )		const;
	virtual uint32_t				GetViewFinal			( void )		const;
    void                            SetListBoxScrollIndex(uint32_t index);

	/*
	load & save & clone
	*/
    virtual void                           Load         ( ui_data_control_t control );

	/*
	virtual
	*/
	virtual void				PerformLayout			( ui_vector_2_t client_sz );


protected:
	/*
	event
	*/
    static void on_mouse_rise(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);    
    static void on_vscroll_changed(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void setup(plugin_ui_control_meta_t meta);

	/*
	method
	*/
	virtual void				UpdateSelf				( float deltaTime );
	virtual void				UpdateScrollRange		( ui_vector_2_t client_sz );
    
protected:
	/*
	member
	*/
	uint32_t	mColCount;

	/*
	hidden destructor
	*/
	virtual ~RGUIListBoxCol( void ){}
    friend class RGUIControlRepo; 
};

#endif//__RGUILISTBOXCOL_H__
