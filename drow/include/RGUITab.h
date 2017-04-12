#ifndef DROW_UI_CONTROL_TAB_H
#define DROW_UI_CONTROL_TAB_H
#include "RGUIControl.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUITab, ui_control_type_tab);

class RGUITab : public RGUIControl {
public:
	/*
	constructor
	*/
	RGUITab();

	/*
	page
	*/
    void								AddTabPage				( const ::std::string& text );

	/*
	load & save & clone
	*/
    virtual void                        Load                    ( ui_data_control_t control );


protected:
	/*
	call back
	*/
	virtual void OnLoadProperty(void);

	void PerformLayout(ui_vector_2_t client_sz);
	void DoClick(ui_vector_2 const & pt);

    static void on_mouse_down(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    
	/*
	method
	*/
	virtual void						RenderTail				( ui_runtime_render_t ctx, ui_rect_t rect );

	void								TabToggleRender			( ui_runtime_render_t ctx, ui_rect_t rect, RGUITabPage* page, const ui_rect & rt );
	RGUIUnitVec2						CalToggleTemplateUnit	( void ) const;


protected:
	/*
	member
	*/
	uint8_t			mToggleDock;	
	RGUIUnitVec2	mToggleGrapUnit;
    RGUIUnitVec2	mToggleRelaUnit;
    RVector2                           mToggleGrapReal;
    RVector2                           mToggleRelaReal;

	std::string		mToggleTemplateName;
	ui_data_control_t						mToggleTemplate;

    static void on_layout(plugin_ui_control_t control, ui_vector_2_t client_sz);    
    static void setup(plugin_ui_control_meta_t meta);
    
protected:
	/*
	hidden destructor
	*/
	virtual ~RGUITab( void ){};
    friend class RGUIControlRepo; 
};

#endif//__RGUITAB_H__
