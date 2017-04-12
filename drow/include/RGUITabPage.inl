inline
const std::string&
		RGUITabPage::GetToggleTemplateName	( void ) const
{
	return mToggleTemplateName;
}

inline
const ::std::string&
		RGUITabPage::GetToggleText			( void ) const
{
	return mToggleText;
}

inline
void	RGUITabPage::SetToggleText			( const ::std::string& text )
{
	if (mToggleText != text)
	{
		mToggleText  = text;
		mToggleTextKey = 0;
	}
}

inline
uint32_t
		RGUITabPage::GetToggleTextKey		( void ) const
{
	return mToggleTextKey;
}

inline
void	RGUITabPage::SetToggleTextKey		(uint32_t key  )
{
	if (mToggleTextKey != key)
	{
		mToggleTextKey  = key;
		Retext();
	}
}

