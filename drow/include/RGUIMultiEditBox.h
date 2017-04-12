#ifndef DROW_UI_CONTROL_MULTIEDITBOX_H
#define DROW_UI_CONTROL_MULTIEDITBOX_H
#include <vector>
#include "RGUIEditBox.h"

DROW_UI_DEF_CREATABLE_CONTROL(RGUIMultiEditBox, ui_control_type_multi_edit_box);

class RGUIMultiEditBox : public RGUIEditBox {
public:
	/*
	constructor
	*/
	RGUIMultiEditBox();

	/*
	method
	*/
	int32_t							GetLineHeight			( void ) const;
	void							SetLineHeight			( int32_t height );

    virtual void                    Load                    ( ui_data_control_t control );

	/*
	virtual
	*/
	void PerformLayout(ui_vector_2_t client_sz);

protected:
	/*
	call back
	*/
	virtual void OnLoadProperty();
	virtual void OnTextChanged();

private:
	/*
	multiline edit box
	*/
	void							SplitText				( void );
	uint32_t							GetLineIndexByIndex		( uint32_t index ) const;

protected:
	/*
	logic line info
	*/
	class LineInfo
	{
	public:
		uint32_t						start;
		uint32_t						count;
	};

	/*
	typedef
	*/
	typedef std::vector<LineInfo>
		  LineInfoVec;

	/*
	member
	*/
	int32_t		mLineHeight;
	RGUIUnit	mUnitHeight;
	LineInfoVec						mLineInfoVec;

    static void on_layout(plugin_ui_control_t control, ui_vector_2_t client_sz);    
    static void setup(plugin_ui_control_meta_t meta);
    
protected:
	/*
	hidden destructor
	*/
	virtual ~RGUIMultiEditBox( void ){};
    friend class RGUIControlRepo;     
};

#include "RGUIMultiEditBox.inl"

#endif//__RGUIMULTIEDITBOX_H__
