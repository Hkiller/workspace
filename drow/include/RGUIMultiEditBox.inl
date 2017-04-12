inline
int32_t	RGUIMultiEditBox::GetLineHeight	( void ) const
{
	return mLineHeight;
}

inline
void	RGUIMultiEditBox::SetLineHeight	( int32_t height )
{
	if (mLineHeight != height)
	{
		mLineHeight  = height;
        mUnitHeight  = RGUIUnitVec2::ToUnit(
            false, 
            GetRenderRealSZ(), 
            GetEditorRealSZ(), 
            RVector2(0.0f, (float)height)).y;
	}
}
