#include "cpe/pal/pal_math.h"

#if defined CPE_USE_MODFF
float msc_modff(float _X, float *_Y) {
    double _Di, _Df = modf((double)_X, &_Di);
    *_Y = (float)_Di;
    return ((float)_Df);
}
#endif
