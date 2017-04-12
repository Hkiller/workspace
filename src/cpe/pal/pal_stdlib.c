#include "cpe/pal/pal_stdlib.h"

#ifdef _CPE_NO_STRTOF

float strtof(const char * s, char const ** endptr) {
    char * e;
    double b;

    e = NULL;
    b = strtod(s, &e);
    if (endptr) *endptr = e;

    if (*e == 0 && (b < -3.40E+38 || b > +3.40E+38 )) {
        if (endptr) *endptr = s;
    }

    return (float)b;
}

#endif
