#ifndef DROW_UI_CONTROL_TABPAGE_H
#define DROW_UI_CONTROL_TABPAGE_H
#include "RGUIPanel.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUITabPage, ui_control_type_tab_page);

class RGUITabPage : public RGUIPanel {
public:
	/*
	constructor
	*/
	RGUITabPage();

	/*
	method
	*/
	const std::string&				GetToggleTemplateName	( void ) const;
	void							SetToggleTemplateName	( const std::string& name );

	/*
	text
	*/
	const ::std::string&			GetToggleText			( void ) const;
    void							SetToggleText			( const ::std::string&     text );
	uint32_t				        GetToggleTextKey		( void ) const;
	void							SetToggleTextKey		( uint32_t key  );

	/*
	load & save
	*/
    virtual void                    Load                    ( ui_data_control_t control );

	/*
	virtual
	*/
	virtual void					Retext					( void );

protected:
	/*
	call back
	*/
	virtual void OnLoadProperty(void);

public:
	/*
	member
	*/
	::std::string mToggleText;
	uint32_t mToggleTextKey;
	std::string	mToggleTemplateName;
	ui_data_control_t mToggleTemplate;

public:
	/*
	hidden destructor
	*/
	virtual ~RGUITabPage( void ){};
    friend class RGUIControlRepo; 
};

#include "RGUITabPage.inl"

#endif//__RGUITABPAGE_H__
