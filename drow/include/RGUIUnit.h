#ifndef __RGUIUNIT_H__
#define __RGUIUNIT_H__
#include "protocol/render/model/ui_common.h"
#include "RPCH.h"

class RGUIUnit
{
public:
	/*
	member
	*/
	float	k;
	float	b;

	/*
	static
	*/
	static const RGUIUnit	Zero;
	static const RGUIUnit	PositiveUnit;
	static const RGUIUnit	PositiveHalf;
	static const RGUIUnit	NegativeUnit;
	static const RGUIUnit	NegativeHalf;

	/*
	constructor
	*/
	RGUIUnit( void );
	RGUIUnit( float _k );
	RGUIUnit( float _k, float _b );
	RGUIUnit( const RGUIUnit& other );
    RGUIUnit( UI_UNIT const & other )
        : k(other.k)
        , b(other.b)
    {
    }

	/*
	operator
	*/
	RGUIUnit		operator+	( const RGUIUnit& other ) const;
	RGUIUnit		operator-	( const RGUIUnit& other ) const;
	RGUIUnit		operator-	( void ) const;

	RGUIUnit		operator*	( float f ) const;
	RGUIUnit		operator/	( float f ) const;
	RGUIUnit&		operator=	( const RGUIUnit& other );

	RGUIUnit&		operator=	( UI_UNIT const & other ) {
        k = other.k;
        b = other.b;
        return *this;
    }
    
	RGUIUnit&		operator+=	( const RGUIUnit& other );
	RGUIUnit&		operator-=	( const RGUIUnit& other );
	RGUIUnit&		operator*=	( float f );
	RGUIUnit&		operator/=	( float f );

	bool			operator==	( const RGUIUnit& other ) const;
	bool			operator!=	( const RGUIUnit& other ) const;

	bool			operator >	( const RGUIUnit& other ) const;
	bool			operator >=	( const RGUIUnit& other ) const;
	bool			operator <	( const RGUIUnit& other ) const;
	bool			operator <=	( const RGUIUnit& other ) const;

	/*
	method
	*/
	float			ToReal		( float base ) const;
};

#include "RGUIUnit.inl"

#endif//__RGUIUNIT_H__
