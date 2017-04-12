#ifndef CPE_PAL_MATH_H
#define CPE_PAL_MATH_H
#define _USE_MATH_DEFINES
#include <math.h>

#ifndef M_PI_X_2
#define M_PI_X_2 (float)M_PI * 2.0f
#endif

#ifndef INFINITY
	#ifdef _MSC_VER
		union MSVC_EVIL_FLOAT_HACK
		{
			unsigned __int8 Bytes[4];
			float Value;
		};
		static union MSVC_EVIL_FLOAT_HACK INFINITY_HACK = {{0x00, 0x00, 0x80, 0x7F}};
		#define INFINITY (INFINITY_HACK.Value)
	#endif
	
	#ifdef __GNUC__
		#define INFINITY (__builtin_inf())
	#endif
	
	#ifndef INFINITY
		#define INFINITY (1e1000)
	#endif

    #if defined modff
        #undef modff
        #define CPE_USE_MODFF
        float msc_modff(float _X, float *_Y);
        #define modff msc_modff
    #endif
#endif

#endif
