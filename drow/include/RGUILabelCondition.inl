inline
uint32_t	RGUILabelCondition::GetIndex			( void ) const
{
	return mIndex;
}

inline
uint32_t	RGUILabelCondition::GetRenderTextCount	( void ) const
{
	return (uint32_t)mRenderTextVec.size();
}

inline
const RGUILabelCondition::RenderText&
		RGUILabelCondition::GetRenderText		( uint32_t index ) const
{
	assert(index < mRenderTextVec.size());
	return mRenderTextVec[index];
}

