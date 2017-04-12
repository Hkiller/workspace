/*
constructor
*/
inline 
RGUIUnitVec2::RGUIUnitVec2( void )
{}
inline
RGUIUnitVec2::RGUIUnitVec2( const RGUIUnit& _x, const RGUIUnit& _y )
: x(_x)
, y(_y)
{}
inline 
RGUIUnitVec2::RGUIUnitVec2( const RGUIUnitVec2& other )
: x(other.x)
, y(other.y)
{}

/*
operator
*/
inline 
RGUIUnitVec2
		RGUIUnitVec2::operator +	( const RGUIUnitVec2& other ) const
{
	return RGUIUnitVec2(x+other.x, y+other.y);
}
inline
RGUIUnitVec2	
		RGUIUnitVec2::operator -	( const RGUIUnitVec2& other ) const
{
	return RGUIUnitVec2(x-other.x, y-other.y);
}
inline
RGUIUnitVec2
		RGUIUnitVec2::operator *	( float f ) const
{
	return RGUIUnitVec2(x*f, y*f);
}
inline
RGUIUnitVec2	
		RGUIUnitVec2::operator /	( float f ) const
{
	return RGUIUnitVec2(x/f, y/f);
}
inline 
RGUIUnitVec2&	
		RGUIUnitVec2::operator =	( const RGUIUnitVec2& other )
{
	x = other.x;
	y = other.y;
	return (*this);
}
inline
RGUIUnitVec2&	
		RGUIUnitVec2::operator +=	( const RGUIUnitVec2& other )
{
	x += other.x;
	y += other.y;
	return (*this);
}
inline 
RGUIUnitVec2&	
		RGUIUnitVec2::operator -=	( const RGUIUnitVec2& other )
{
	x -= other.x;
	y -= other.y;
	return (*this);
}
inline
RGUIUnitVec2&	
		RGUIUnitVec2::operator *=	( float f )
{
	x *= f;
	y *= f;
	return (*this);
}
inline
RGUIUnitVec2&	
		RGUIUnitVec2::operator /=	( float f )
{
	x /= f;
	y /= f;
	return (*this);
}
inline 
bool	RGUIUnitVec2::operator ==	( const RGUIUnitVec2& other ) const
{
	return x == other.x && y == other.y;
}
inline 
bool	RGUIUnitVec2::operator !=	( const RGUIUnitVec2& other ) const
{
	return x != other.x || y != other.y;
}

/*
method
*/
inline
RGUIUnitVec2& RGUIUnitVec2::LimitMin( const RGUIUnitVec2& min ) {
	x = cpe_max(x, min.x);
	y = cpe_max(y, min.y);
	return (*this);
}

inline
RGUIUnitVec2& RGUIUnitVec2::LimitMax( const RGUIUnitVec2& max ) {
	x = cpe_min(x, max.x);
	y = cpe_min(y, max.y);
	return (*this);
}

inline
RVector2
        RGUIUnitVec2::ToReal       ( bool uers, const RVector2& curr, const RVector2& orig ) const
{
    RVector2 base = ToBase(uers, curr, orig);
    return RVector2(
        x.ToReal(base.w),
        y.ToReal(base.h));
}

inline
RVector2
        RGUIUnitVec2::ToBase       ( bool uers, const RVector2& curr, const RVector2& orig )
{
    RVector2 base = curr;
    if (!uers)
    {
        float wr   = curr.w / orig.w;
        float hr   = curr.h / orig.h;
        if (wr < hr) base.h = orig.h * wr;
        if (wr > hr) base.w = orig.w * hr;

        float min  = cpe_min(base.w, base.h);
        base.w     = min;
        base.h     = min;
    }

    return base;
}

inline
RGUIUnitVec2
        RGUIUnitVec2::ToUnit       ( bool uers, const RVector2& curr, const RVector2& orig, const RVector2& real )
{
    RVector2 base = ToBase(uers, curr, orig);
    return RGUIUnitVec2(
        RGUIUnit(real.w / base.w),
        RGUIUnit(real.h / base.h));
}
