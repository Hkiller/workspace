#ifndef __RGUIUNITVEC2_H__
#define __RGUIUNITVEC2_H__
#include "RVector2.h"
#include "RGUIUnit.h"

class RGUIUnitVec2
{
public:
	/*
	member
	*/
	RGUIUnit	x;
	RGUIUnit	y;

	/*
	static
	*/
	static const RGUIUnitVec2 Zero;
	static const RGUIUnitVec2 PositiveOne;
	static const RGUIUnitVec2 NegativeOne;

	/*
	constructor
	*/
	RGUIUnitVec2( void );
	RGUIUnitVec2( const RGUIUnit& _x, const RGUIUnit& _y );
	RGUIUnitVec2( const RGUIUnitVec2& other );
    RGUIUnitVec2( const UI_UNIT_VECTOR_2& other)
        : x(other.x)
        , y(other.y)
    {
    }

	/*
	operator
	*/
	RGUIUnitVec2			operator +	( const RGUIUnitVec2& other ) const;
	RGUIUnitVec2			operator -	( const RGUIUnitVec2& other ) const;
	RGUIUnitVec2			operator *	( float f ) const;
	RGUIUnitVec2			operator /	( float f ) const;
	RGUIUnitVec2&			operator =	( const RGUIUnitVec2& other );
    
	RGUIUnitVec2&			operator =	( const UI_UNIT_VECTOR_2& other ) {
        x = other.x;
        y = other.y;
        return *this;
    }
    
	RGUIUnitVec2&			operator +=	( const RGUIUnitVec2& other );
	RGUIUnitVec2&			operator -=	( const RGUIUnitVec2& other );
	RGUIUnitVec2&			operator *=	( float f );
	RGUIUnitVec2&			operator /=	( float f );

	bool					operator == ( const RGUIUnitVec2& other ) const;
	bool					operator != ( const RGUIUnitVec2& other ) const;

	/*
	method
	*/
	RGUIUnitVec2&			LimitMin	( const RGUIUnitVec2& min );
	RGUIUnitVec2&			LimitMax	( const RGUIUnitVec2& max );

	/*
	real
	*/
	RVector2				ToReal		( bool uers, const RVector2& curr, const RVector2& orig ) const;

    /*
    base
    */
    static RVector2        ToBase      ( bool uers, const RVector2& curr, const RVector2& orig );

	/*
	unit
	*/
	static RGUIUnitVec2	ToUnit		( bool uers, const RVector2& curr, const RVector2& orig, const RVector2& real );
};

#include "RGUIUnitVec2.inl"

#endif//__RGUIUNITVEC2_H__
