#ifndef  __RVECTOR2_H__
#define  __RVECTOR2_H__
#include "protocol/render/model/ui_common.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_vector_2.h"
#include "cpe/utils/math_ex.h"

class RVector2: public ui_vector_2
{
public:
	/*
	member
	*/

public:
	/*
	static data
	*/
	static const RVector2 Zero;			/**<  0.0,  0.0 */
	static const RVector2 PositiveOne;		/**<  1.0,  1.0 */
	static const RVector2 NegativeOne;		/**< -1.0, -1.0 */
	static const RVector2 PositiveUnitX;	/**<  1.0,  0.0 */
	static const RVector2 PositiveUnitY;	/**<  0.0,  1.0 */
	static const RVector2 NegativeUnitX;	/**< -1.0,  0.0 */
	static const RVector2 NegativeUnitY;	/**<  0.0, -1.0 */

public:
	/*
	constructor
	*/
	RVector2( void );
	RVector2( float _x, float _y );
	RVector2( const float* _value );
	RVector2( const ui_vector_2& other );
    RVector2( UI_VECTOR_2 const & other ) {
        x = other.value[0];
        y = other.value[1];
    }

	/*
	operator assignment 
	*/
	RVector2&		operator =				( const float* _value );
	RVector2&		operator =				( const RVector2& other );
	RVector2&		operator =				( const UI_VECTOR_2& other ) {
        x = other.value[0];
        y = other.value[1];
        return *this;
    }

	/*
	operator compare
	*/
	bool			operator ==				( const RVector2& other ) const;
	bool			operator !=				( const RVector2& other ) const;
	bool			operator <				( const RVector2& other ) const;
	bool			operator >				( const RVector2& other ) const;
	bool			operator <=				( const RVector2& other ) const;
	bool			operator >=				( const RVector2& other ) const;

	/*
	operator
	*/
	RVector2		operator +				( const RVector2& other ) const;
	RVector2		operator +				( float f ) const;
	RVector2&		operator +=				( const RVector2& other );
	RVector2&		operator +=				( float f );
	RVector2		operator -				( const RVector2& other ) const;
	RVector2		operator -				( float f ) const;
	RVector2&		operator -=				( const RVector2& other );
	RVector2&		operator -=				( float f );
	RVector2		operator -				( void ) const;

	/*
	operator multiply
	*/
	RVector2		operator *				( const RVector2& other ) const;
	RVector2		operator *				( float f ) const;
	RVector2&		operator *=				( const RVector2& other );
	RVector2&		operator *=				( float f );

	/*
	operator divide
	*/
	RVector2		operator /				( const RVector2& other ) const;
	RVector2		operator /				( float f ) const;
	RVector2&		operator /=				( const RVector2& other );
	RVector2&		operator /=				( float f );

	/*
	operator get&set
	*/
					operator float*			( void );
					operator const float*	( void ) const;
	float&			operator []				( int32_t index );
	const float&	operator []				( int32_t index ) const;


	/*
	method
	*/
	RVector2		GetInvert				( void ) const;
	RVector2&		SetInvert				( void );

	RVector2		GetNormalize			( void ) const;
	RVector2&		SetNormalize			( void );

	float			GetLength				( void ) const;
	float			GetLengthSquare			( void ) const;
	
	float			GetLength				( const RVector2& other ) const;
	float			GetLengthSquare			( const RVector2& other ) const;

	float			GetDot					( const RVector2& other ) const;
	float			GetAngle				( const RVector2& other ) const;

	RVector2&		LimitMin				( const RVector2& min );
	RVector2&		LimitMax				( const RVector2& max );
};

#include "RVector2.inl"

#endif//__RVECTOR2_H__
