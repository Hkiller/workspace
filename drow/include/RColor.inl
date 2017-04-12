inline 
RColor::RColor( void ) {
    r = 1.0f;
    g = 1.0f;
    b = 1.0f;
    a = 1.0f;
}

inline 
RColor::RColor( float _r, float _g, float _b ) {
    r = _r;
    g = _g;
    b = _b;
    a = 1.0f;
}

inline 
RColor::RColor( float _r, float _g, float _b, float _a ) {
    r = _r;
    g = _g;
    b = _b;
    a = _a;
}

inline 
RColor::RColor( uint32_t argb )
{
	SetARGB(argb);
}

inline 
RColor::RColor( const ui_color& other ) {
    r = other.r;
    g = other.g;
    b = other.b;
    a = other.a;
}

/*
operator
*/
inline
RColor&	RColor::operator =		( const RColor& other )
{
	r = other.r;
	g = other.g;
	b = other.b;
	a = other.a;

	return *this;
}

inline 
bool		RColor::operator ==	( const RColor& other ) const
{
	return (r == other.r && 
			g == other.g && 
			b == other.b && 
			a == other.a);
}
inline 
bool		RColor::operator !=	( const RColor& other ) const
{
	return (r != other.r ||
			g != other.g || 
			b != other.b || 
			a != other.a);
}

/*
operator multiple
*/
inline
RColor		RColor::operator *		( float f ) const
{
	return  RColor(r*f, g*f, b*f, a*f);
}
inline
RColor&	RColor::operator *=	( float f )
{
	r *= f;
	g *= f;
	b *= f;
	a *= f;
	return (*this);
}

inline
RColor		RColor::operator *		( const RColor& other ) const
{
	return  RColor(r*other.r, g*other.g, b*other.b, a*other.a);
}
inline
RColor&	RColor::operator *=	( const RColor& other )
{
	r *= other.r;
	g *= other.g;
	b *= other.b;
	a *= other.a;
	return (*this);
}

/*
operator
*/
inline
RColor		RColor::operator +		( const RColor& other ) const
{
	return  RColor(r+other.r, g+other.g, b+other.b, a+other.a);
}
inline
RColor		RColor::operator -		( const RColor& other ) const
{
	return  RColor(r-other.r, g-other.g, b-other.b, a-other.a);
}
inline
RColor&	RColor::operator +=	( const RColor& other )
{
	r += other.r;
	g += other.g;
	b += other.b;
	a += other.a;
	return (*this);
}
inline
RColor&	RColor::operator -=	( const RColor& other )
{
	r -= other.r;
	g -= other.g;
	b -= other.b;
	a -= other.a;
	return (*this);
}

//method
inline
uint32_t		RColor::GetABGR		( void ) const
{
	return ((uint32_t)(a*255.0f)<<24 | (uint32_t)(b*255.0f)<<16 | (uint32_t)(g*255.0f)<<8 | (uint32_t)(r*255.0f));
}

inline 
uint32_t		RColor::GetARGB		( void ) const
{
	return ((uint32_t)(a*255.0f)<<24 | (uint32_t)(r*255.0f)<<16 | (uint32_t)(g*255.0f)<<8 | (uint32_t)(b*255.0f));
}

inline 
void		RColor::SetARGB		( uint32_t argb )
{
	static const float inv = 1.0f/255.0f;
	a = (float)((argb & 0xFF000000)>>24) * inv;
	r = (float)((argb & 0x00FF0000)>>16) * inv;
	g = (float)((argb & 0x0000FF00)>>8)  * inv;
	b = (float)((argb & 0x000000FF))     * inv;
}

inline
void		RColor::SetRGBA		( uint32_t rgba )
{
	static const float inv = 1.0f/255.0f;
	r = (float)((rgba & 0xFF000000)>>24) * inv;
	g = (float)((rgba & 0x00FF0000)>>16) * inv;
	b = (float)((rgba & 0x0000FF00)>>8)  * inv;
	a = (float)((rgba & 0x000000FF))     * inv;
}

inline 
RColor		RColor::GetRGB			( void ) const
{
	return  RColor(r, g, b);
}

inline 
void		RColor::SetARGB		( float _r, float _g, float _b, float _a )
{
	r = _r;
	g = _g;
	b = _b;
	a = _a;
}

inline 
void		RColor::SetRGB			( float _r, float _g, float _b )
{
	r = _r;
	g = _g;
	b = _b;
}

inline 
void		RColor::SetAlpha		( float _a )
{
	a = _a;
}
