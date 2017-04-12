#ifndef __RCOLOR_H__
#define __RCOLOR_H__
#include "protocol/render/model/ui_common.h"
#include "render/utils/ui_color.h"
#include "RPCH.h"

class RColor: public ui_color {
public:
	/*
	constructor
	*/
	RColor( void );
	RColor( float _r, float _g, float _b );
	RColor( float _r, float _g, float _b, float _a );
	RColor( uint32_t argb );
	RColor( const ui_color& other );
    RColor( const UI_COLOR& other ) {
        a = other.a;
        r = other.r;
        g = other.g;
        b = other.b;
    }

	/*
	operator assignment
	*/
	RColor&	operator =		( const RColor& other );

	RColor&	operator =		( const UI_COLOR& other ) {
        a = other.a;
        r = other.r;
        g = other.g;
        b = other.b;
        return *this;
    }
    
	bool		operator ==		( const RColor& other ) const;
	bool		operator !=		( const RColor& other ) const;

	/*
	operator multiple
	*/
	RColor		operator *		( float f ) const;
	RColor&	operator *=		( float f );
	RColor		operator *		( const RColor& other ) const;
	RColor&	operator *=		( const RColor& other );

	/*
	operator
	*/
	RColor		operator +		( const RColor& other ) const;
	RColor		operator -		( const RColor& other ) const;
	RColor&	operator +=		( const RColor& other );
	RColor&	operator -=		( const RColor& other );

	/*
	method
	*/
	uint32_t		GetABGR			( void ) const;
	uint32_t		GetARGB			( void ) const;
	void		SetARGB			( uint32_t argb );
	void		SetRGBA			( uint32_t rgba );

	RColor		GetRGB			( void ) const;
	void		SetARGB			( float _r, float _g, float _b, float _a );
	void		SetRGB			( float _r, float _g, float _b );
	void		SetAlpha		( float _a );
};

#include "RColor.inl"

#endif//__RCOLOR_H__
