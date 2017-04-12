inline
int32_t	RGUIListBoxBase::GetHorzGrap			( void ) const
{
	return mHorzGrap;
}

inline
void	RGUIListBoxBase::SetHorzGrap			( int32_t horzGrap )
{
    mHorzGrap = horzGrap;
}

inline
int32_t	RGUIListBoxBase::GetVertGrap			( void ) const
{
	return mVertGrap;
}

inline
void	RGUIListBoxBase::SetVertGrap			( int32_t vertGrap )
{
    mVertGrap = vertGrap;
}


inline
bool	RGUIListBoxBase::WasLightFrameShow		( void ) const
{
	return mLightFrameShow;
}

inline
void	RGUIListBoxBase::SetLightFrameShow		( bool flag )
{
	if (mLightFrameShow != flag)
	{
		mLightFrameShow  = flag;
	}
}

inline
void	RGUIListBoxBase::SetItemCreator		( ItemCreator creator )
{
	mItemCreator = creator;
}

inline
ui_data_control_t
		RGUIListBoxBase::GetItemTemplate		( void ) 
{
	return mItemTemplate;
}

inline
ui_data_control_t
		RGUIListBoxBase::GetHeadTemplate		( void )
{
	return mHeadTemplate;
}

inline
ui_data_control_t
		RGUIListBoxBase::GetTailTemplate		( void )
{
	return mTailTemplate;
}

inline
RGUIListBoxAdvItem*
		RGUIListBoxBase::GetSelectedItem		( void )
{
	return GetItem(GetSelectedItemIndex());
}

inline
uint32_t	RGUIListBoxBase::GetSelectedItemIndex	( void ) const
{
	for (uint32_t i = 0; i < mItemList.size(); ++i)
		if (mItemList[i].selected)
			return i;

	return -1;
}

inline
RGUIListBoxAdvItem*
		RGUIListBoxBase::GetHead				( void )
{
	return mHeadCtrl;
}

inline
bool	RGUIListBoxBase::WasHeadShow			( void ) const
{
	return mHeadShow;
}

inline
void	RGUIListBoxBase::SetHeadShow			( bool flag )
{
	mHeadShow = flag;
}

inline
RGUIListBoxAdvItem*
		RGUIListBoxBase::GetTail				( void )
{
	return mTailCtrl;
}

inline
bool	RGUIListBoxBase::WasTailShow			( void ) const
{
	return mTailShow;
}

inline
void	RGUIListBoxBase::SetTailShow			( bool flag )
{
	mTailShow = flag;
}

inline
uint32_t	RGUIListBoxBase::GetItemCount			( void ) const
{
	return (uint32_t)mItemList.size();
}
