/*
constructor
*/
inline 
RVector2::RVector2( void ) {
    x = 0.0f;
    y = 0.0f;
}

inline
RVector2::RVector2( float _x, float _y ) {
    x = _x;
    y = _y;
}

inline
RVector2::RVector2( const float* _value ) {
    x = _value[0];
    y = _value[1];
}

inline
RVector2::RVector2( const ui_vector_2& other ) {
    x = other.x;
    y = other.y;
}

/*
method
*/
inline
RVector2&		RVector2::operator =			( const float* _value )
{
	x = _value[0];
	y = _value[1];
	return (*this);
}
inline
RVector2&		RVector2::operator =			( const RVector2& other )
{
	x = other.x;
	y = other.y;
	return (*this);
}

inline
bool			RVector2::operator ==			( const RVector2& other ) const
{
	return (x == other.x && 
			y == other.y);
}
inline
bool			RVector2::operator !=			( const RVector2& other ) const
{
	return (x != other.x || 
			y != other.y);
}
inline
bool			RVector2::operator <			( const RVector2& other ) const
{
	return (x != other.x) ? x <  other.x : y <  other.y;
}
inline
bool			RVector2::operator <=			( const RVector2& other ) const
{
	return (x != other.x) ? x <= other.x : y <= other.y;
}
inline
bool			RVector2::operator >			( const RVector2& other ) const
{
	return (x != other.x) ? x >  other.x : y >  other.y;
}
inline
bool			RVector2::operator >=			( const RVector2& other ) const
{
	return (x != other.x) ? x >= other.x : y >= other.y;
}

inline
RVector2		RVector2::operator +			( const RVector2& other ) const
{
	return RVector2(x+other.x, y+other.y);
}
inline
RVector2		RVector2::operator +			( float f ) const
{
	return RVector2(x+f, y+f);
}
inline
RVector2&		RVector2::operator +=			( const RVector2& other ) 
{
	x += other.x;
	y += other.y;
	return (*this);
}
inline
RVector2&		RVector2::operator +=			( float f )
{
	x += f;
	y += f;
	return (*this);
}
inline
RVector2		RVector2::operator -			( const RVector2& other ) const
{
	return RVector2(x-other.x, y-other.y);
}
inline
RVector2		RVector2::operator -			( float f ) const
{
	return RVector2(x-f, y-f);
}
inline
RVector2&		RVector2::operator -=			( const RVector2& other )
{
	x -= other.x;
	y -= other.y;
	return (*this);
}
inline
RVector2&		RVector2::operator -=			( float f )
{
	x -= f;
	y -= f;
	return (*this);
}
inline
RVector2		RVector2::operator -			( void ) const
{
	return RVector2(-x, -y);
}


inline
RVector2		RVector2::operator *			( const RVector2& other ) const
{
	return RVector2(x*other.x, y*other.y);
}
inline
RVector2		RVector2::operator *			( float f ) const
{
	return RVector2(x*f, y*f);
}
inline
RVector2&		RVector2::operator *=			( const RVector2& other )
{
	x *= other.x;
	y *= other.y;
	return (*this);
}
inline
RVector2&		RVector2::operator *=			( float f )
{
	x *= f;
	y *= f;
	return (*this);
}


inline
RVector2		RVector2::operator /			( const RVector2& other ) const
{
	return RVector2(x/other.x, y/other.y);
}
inline
RVector2		RVector2::operator /			( float f ) const
{
	return RVector2(x/f, y/f);
}
inline
RVector2&		RVector2::operator /=			( const RVector2& other ) 
{
	x /= other.x;
	y /= other.y;
	return (*this);
}
inline
RVector2&		RVector2::operator /=			( float f )
{
	x /= f;
	y /= f;
	return (*this);
}


inline
				RVector2::operator float*		( void )
{
	return value;
}
inline
				RVector2::operator const float*( void ) const
{
	return value;
}
inline
float&			RVector2::operator []			( int32_t index )
{
	return value[index];
}
inline
const float&	RVector2::operator []			( int32_t index ) const
{
	return value[index];
}

/*
method
*/
inline
RVector2		RVector2::GetInvert			( void ) const
{
	return RVector2(-x, -y);
}
inline
RVector2&		RVector2::SetInvert			( void )
{
	x = -x;
	y = -y;
	return (*this);
}
inline
RVector2		RVector2::GetNormalize			( void ) const
{
	float length = GetLength();
	if (length)
	{
		float  invLength = 1.0f/length;
		return RVector2(x*invLength, y*invLength);
	}

	return RVector2::Zero;
}
inline
RVector2&		RVector2::SetNormalize			( void )
{
	return (*this) = GetNormalize();
}

inline
float			RVector2::GetLength			( void ) const
{
	return sqrt(GetLengthSquare());
}
inline
float			RVector2::GetLengthSquare		( void ) const
{
	return x*x + y*y;
}
inline
float			RVector2::GetLength			( const RVector2& other ) const
{
	return sqrt(GetLengthSquare(other));
}
inline
float			RVector2::GetLengthSquare		( const RVector2& other ) const
{
	float tempX = other.x - x;
	float tempY = other.y - y;

	return tempX*tempX + 
		   tempY*tempY;
}
inline
float			RVector2::GetDot				( const RVector2& other ) const
{
	return x*other.x + y*other.y;
}
inline
float			RVector2::GetAngle				( const RVector2& other ) const
{
	return acos(GetDot(other)/(GetLength()*other.GetLength()));
}

inline
RVector2&		RVector2::LimitMin				( const RVector2& min )
{
	w = cpe_max(w, min.w);
	h = cpe_max(h, min.h);
	return (*this);
}
inline
RVector2&		RVector2::LimitMax				( const RVector2& max )
{
	w = cpe_min(w, max.w);
	h = cpe_min(h, max.h);
	return (*this);
}
