#ifndef DROW_UI_CONTROL_LISTBOXBASE_H
#define DROW_UI_CONTROL_LISTBOXBASE_H
#include <string>
#include <vector>
#include "RGUIScrollable.h"
#include "RGUIListBoxAdvItem.h"

class RGUIListBoxBase : public RGUIScrollable {
public:
	/*
	typedef
	*/
	typedef RGUIListBoxAdvItem* (*ItemCreator) ( void );

	/*
	constructor
	*/
	RGUIListBoxBase();

	/*
	method
	*/
	int32_t								GetHorzGrap					( void ) const;
    void	                            SetHorzGrap			        ( int32_t horzGrap );
	int32_t								GetVertGrap					( void ) const;
    void	                            SetVertGrap			        ( int32_t vertGrap );

	plugin_ui_control_frame_t	        GetLightFrame				( void ) const;
	void								SetLightFrame				( const char * res );

	bool								WasLightFrameShow			( void ) const;
	void								SetLightFrameShow			( bool flag );

	/*
	item template
	*/
	void								SetItemCreator				( ItemCreator creator );
	ui_data_control_t				GetItemTemplate				( void );
	ui_data_control_t			GetHeadTemplate				( void );
	ui_data_control_t				GetTailTemplate				( void );

	/*
	select item
	*/
	RGUIListBoxAdvItem*				GetSelectedItem				( void );
	void								SetSelectedItem				( uint32_t index, bool flag );
	uint32_t							GetSelectedItemIndex			( void ) const;

	/*
	head
	*/
	RGUIListBoxAdvItem*				GetHead						( void );
	bool								WasHeadShow					( void ) const;
	void								SetHeadShow					( bool flag );

	/*
	rail
	*/
	RGUIListBoxAdvItem*				GetTail						( void );
	bool								WasTailShow					( void ) const;
	void								SetTailShow					( bool flag );

	/*
	item
	*/
	RGUIListBoxAdvItem*				GetItem						( RGUIControl* ctrl );
	RGUIListBoxAdvItem*				GetItem						( uint32_t index );
	void								AddItem						( uint32_t index );
	void								AddItem						( uint32_t index, uint32_t count );
	void								DelItem						( uint32_t index );
	void								DelItem						( uint32_t index, uint32_t count );
	void								DelItemAll					( void );
	void								UpdateItemAll					( void );
	void								SetItemAllVisible				( bool visible );
	uint32_t							GetItemCount					( void )		const;
	virtual uint32_t						GetItemIndex					( int32_t point )	const = 0;
	virtual uint32_t						GetViewStart					( void )		const = 0;
	virtual uint32_t						GetViewFinal					( void )		const = 0;

	/*
	item pixel
	*/
	virtual int32_t						GetItemPixel					( void )		const = 0;
	virtual int32_t                       GetItemPixel					( uint32_t index ) const;
	void								SetItemPixel					( uint32_t index, int32_t pixel );

	/*
	load & save & clone
	*/
    void Load(UI_CONTROL_BOX const & data);
    using RGUIScrollable::Load;

protected:
	virtual void OnLoadProperty ();
    void onLineUp(void);
    void onLineDown(void);
    void onLineLeft(void);
    void onLineRight(void);

protected:
    static void on_layout(plugin_ui_control_t control, ui_vector_2_t client_sz);
    static void on_mouse_click(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void on_vscroll_changed(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void on_hscroll_changed(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event);
    static void setup(plugin_ui_control_meta_t meta);

protected:
	void								ItemTackback					( void );
	void								ItemAllocate					( void );
	void								ItemAllocate					( uint32_t index );

protected:
	/*
	item information
	*/
	class ItemInfo {
	public:
		int32_t		pixel;
		bool		selected;
	};

	/*
	member
	*/
	int32_t			mHorzGrap;
	int32_t			mVertGrap;
	plugin_ui_control_frame_t mLightFrame;
	bool			mLightFrameShow;

	bool			mItemFreeSize;
	uint32_t		mItemTempID;
	uint32_t		mHeadTempID;
	uint32_t		mTailTempID;

	ItemCreator							mItemCreator;
	ui_data_control_t mItemTemplate;
	ui_data_control_t mHeadTemplate;
	ui_data_control_t mTailTemplate;

	RGUIListBoxAdvItem * mHeadCtrl;
	RGUIListBoxAdvItem * mTailCtrl;
	bool mHeadShow;
	bool mTailShow;

	std::vector<RGUIListBoxAdvItem*>	mItemPool;
	std::vector<ItemInfo>				mItemList;

protected:
	virtual ~RGUIListBoxBase( void );
};

#include "RGUIListBoxBase.inl"

#endif//__RGUILISTBOXBASE_H__
