#include "cpe/utils/math_ex.h"

#define SC_PRECISION (0.5f)
#define SC_INV_PREC  (1.0f/SC_PRECISION)
#define SC_PERIOD    (int32_t)( 360.0f * SC_INV_PREC )

static uint8_t g_math_ex_fast_init = 0;
static float g_math_ex_fast_sin_lut[SC_PERIOD];
static float g_math_ex_fast_cos_lut[SC_PERIOD];

void cpe_math_fast_calc_init(void) {
    int i;

    if (g_math_ex_fast_init) return;
    
    for(i = 0; i < SC_PERIOD; ++i) {
        g_math_ex_fast_sin_lut[i] = sin(cpe_math_angle_to_radians(((float)i) * SC_PRECISION));
        g_math_ex_fast_cos_lut[i] = cos(cpe_math_angle_to_radians(((float)i) * SC_PRECISION));
    }

    g_math_ex_fast_init = 1;
}

float cpe_fast_sin( float radians ) {
	int32_t theta = (int32_t)(radians * SC_INV_PREC) % SC_PERIOD;
	return theta < 0
        ? - g_math_ex_fast_sin_lut[ -theta ]
        : g_math_ex_fast_sin_lut[ theta ];
}

float cpe_fast_cos( float radians ) {
	int32_t theta = (int32_t)(radians * SC_INV_PREC) % SC_PERIOD;
	return theta < 0
        ? - g_math_ex_fast_cos_lut[ -theta ]
        : g_math_ex_fast_cos_lut[ theta ];
}
