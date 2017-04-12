#ifndef CPE_DR_BUILD_INBUILD_ERROR_H
#define CPE_DR_BUILD_INBUILD_ERROR_H
#include "cpe/dr/dr_metalib_build.h"
#include "cpe/dr/dr_error.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define DR_NOTIFY_ERROR_EXTRA(em, e, i) CPE_ERROR_EX(em, e, "%s: %s", dr_error_string(e), (i)?(i):"");
#define DR_NOTIFY_ERROR(em, e) CPE_ERROR_EX(em, e, "%s", dr_error_string(e));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


