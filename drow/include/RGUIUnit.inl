/*
constrctor
*/
inline
RGUIUnit::RGUIUnit( void )
{}
inline
RGUIUnit::RGUIUnit( float _k )
: k(_k)
, b(0.0f)
{}
inline 
RGUIUnit::RGUIUnit( float _k, float _b )
: k(_k)
, b(_b)
{}
inline
RGUIUnit::RGUIUnit( const RGUIUnit& other )
: k(other.k)
, b(other.b)
{}

/*
operator
*/
inline
RGUIUnit	
		RGUIUnit::operator+	( const RGUIUnit& other ) const
{
	return RGUIUnit(k+other.k, b+other.b);
}
inline 
RGUIUnit
		RGUIUnit::operator-	( const RGUIUnit& other ) const
{
	return RGUIUnit(k-other.k, b-other.b);
}
inline
RGUIUnit
		RGUIUnit::operator-	( void ) const
{
	return RGUIUnit(-k, -b);
}
inline
RGUIUnit
		RGUIUnit::operator*	( float f ) const
{
	return RGUIUnit(k*f, b*f);
}
inline
RGUIUnit
		RGUIUnit::operator/	( float f ) const
{
	return RGUIUnit(k/f, b/f);
}
inline 
RGUIUnit&	
		RGUIUnit::operator=	( const RGUIUnit& other )
{
	k = other.k;
	b = other.b;
	return (*this);
}
inline
RGUIUnit&	
		RGUIUnit::operator+=	( const RGUIUnit& other )
{
	k += other.k;
	b += other.b;
	return (*this);
}
inline
RGUIUnit&
		RGUIUnit::operator-=	( const RGUIUnit& other )
{
	k -= other.k;
	b -= other.b;
	return (*this);
}
inline
RGUIUnit&	
		RGUIUnit::operator*=	( float f )
{
	k *= f;
	b *= f;
	return (*this);
}
inline
RGUIUnit&	
		RGUIUnit::operator/=	( float f )
{
	k /= f;
	b /= f;
	return (*this);
}
inline
bool	RGUIUnit::operator==	( const RGUIUnit& other ) const
{
	return k == other.k && b == other.b;
}
inline
bool	RGUIUnit::operator!=	( const RGUIUnit& other ) const
{
	return k != other.k || b != other.b;
}
inline
bool	RGUIUnit::operator>	( const RGUIUnit& other ) const
{
	if (k >  other.k)
		return true;
	if (k == other.k)
		return b > other.b;

	return false;
}
inline
bool	RGUIUnit::operator>=	( const RGUIUnit& other ) const
{
	return (*this) > other || (*this) == other;
}

inline
bool	RGUIUnit::operator<	( const RGUIUnit& other ) const
{
	if (k <  other.k)
		return true;
	if (k == other.k)
		return b < other.b;

	return false;
}

inline
bool	RGUIUnit::operator<=	( const RGUIUnit& other ) const
{
	return (*this) < other || (*this) == other;
}

/*
method
*/
inline
float	RGUIUnit::ToReal		( float base ) const
{
	float temp = k * base + b;
	return ui_pixel_aligned(temp);
}
