#ifndef CPE_TL_INTERCEPT_H
#define CPE_TL_INTERCEPT_H
#include "tl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*intercept operations*/
tl_intercept_t
tl_intercept_create(
    tl_t tl,
    const char * name,
    tl_intercept_fun_t intercept_fun,
    void * intercept_ctx);

void tl_intercept_free(
    tl_intercept_t intercept);

tl_intercept_t
tl_intercept_find(tl_t tl, const char * name);

#ifdef __cplusplus
}
#endif

#endif
