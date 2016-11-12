#include "cpe/pal/pal_stdarg.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/assert_ex.h"

#ifdef _MSC_VER
#include <windows.h>
#endif

void __cpe_assert_msg(const char *condition, const char *file, int line, int isError, int isHardError, const char *message, ...) {
    va_list vargs;

    fprintf(stderr, (isError ? "Aborting due to error: " : "warning: "));
	
	va_start(vargs, message); {
		vfprintf(stderr, message, vargs);
		fprintf(stderr, "\n");
	} va_end(vargs);
	
	fprintf(stderr, "\tFailed condition: %s\n", condition);
	fprintf(stderr, "\tSource:%s:%d\n", file, line);
}
