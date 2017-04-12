inline
bool	RGUIEditBox::WasMaxLength		( void ) const
{		
	//始终不能超过1024个字符 
	static const uint32_t MAX_LENGTH = 1024;

	return (mMaxLength != (uint32_t)-1 && GetTextLenA() >= mMaxLength)
		|| (mMaxLength == (uint32_t)-1 && GetTextLenA() >= MAX_LENGTH);
}

inline 
uint32_t	RGUIEditBox::GetMaxLength		( void ) const
{
	return mMaxLength;
}

inline 
void	RGUIEditBox::SetMaxLength		( uint32_t length )
{
	mMaxLength = length;
}

inline 
bool	RGUIEditBox::WasPassword		( void ) const
{
	return mPassword;
}

inline 
void	RGUIEditBox::SetPassword		( bool flag )
{
	mPassword = flag;
}

inline 
bool	RGUIEditBox::WasReadOnly		( void ) const
{
	return mReadOnly;
}

inline 
void	RGUIEditBox::SetReadOnly		( bool flag )
{
	mReadOnly = flag;
}

inline 
bool	RGUIEditBox::WasNumberOnly		( void ) const
{
	return mNumberOnly;
}

inline 
void	RGUIEditBox::SetNumberOnly		( bool flag )
{
	mNumberOnly = flag;
}

inline
const ::std::string&
		RGUIEditBox::GetHintText		( void ) const
{
	return mHintText;
}

inline
void	RGUIEditBox::SetHintText		( const ::std::string& text )
{
	mHintText  = text;
}
