#ifndef DROW_UI_CONTROL_LABELCONDITION_H
#define DROW_UI_CONTROL_LABELCONDITION_H
#include <string>
#include <vector>
#include "RGUILabel.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUILabelCondition, ui_control_type_label_condition);

class RGUILabelCondition : public RGUILabel {
public:
	/*
	render text
	*/
	class RenderText {
	public:
		::std::string text;
		uint32_t tkey;
		plugin_layout_font_draw textDraw;
	};

	/*
	typedef
	*/
	typedef std::vector<RenderText> RenderTextVec;

	/*
	constructor
	*/
	RGUILabelCondition();

	/*
	method
	*/
	uint32_t						GetIndex			( void ) const;
	virtual void				SetIndex			( uint32_t index );

	uint32_t						GetRenderTextCount	( void ) const;
	const RenderText&			GetRenderText		( uint32_t index ) const;
	void						DelRenderText		( uint32_t index );
	void						SetRenderText		( uint32_t index, const RenderText& text );
	void						AddRenderText		( const RenderText& text );
	void						DelRenderTextAll	( void );

	/*
	load & save & clone
	*/
    virtual void                Load                ( ui_data_control_t control );

	/*
	virtual
	*/
	virtual void				Retext				( void );

protected:
	/*
	member
	*/
	uint32_t	mIndex;
	RenderTextVec				mRenderTextVec;

protected:
	/*
	hidden destructor
	*/
	virtual ~RGUILabelCondition( void ){};
    friend class RGUIControlRepo; 
};

#include "RGUILabelCondition.inl"

#endif//__RGUILABELCONDITION_H__
