inline
uint16_t	RGUIRadioBox::GetGroup	( void ) const
{
	return mGroup;
}

inline
void	RGUIRadioBox::SetGroup	( uint16_t group )
{
	if (mGroup != group)
	{
		mGroup  = group;
		SetChecked(mChecked);
	}
}
