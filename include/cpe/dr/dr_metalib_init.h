#ifndef CPE_DR_METALIB_INIT_H
#define CPE_DR_METALIB_INIT_H
#include "cpe/dr/dr_types.h"
#include "cpe/dr/dr_define.h"
#include "cpe/dr/dr_external.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CPE_DR_API size_t dr_lib_size(LPDRMETALIB pstLib);
CPE_DR_API const char *dr_lib_name(LPDRMETALIB a_pstLib);
CPE_DR_API int dr_lib_version(LPDRMETALIB a_pstLib);
CPE_DR_API int dr_lib_tag_set_version(LPDRMETALIB a_pstLib);
CPE_DR_API int dr_lib_build_version(LPDRMETALIB a_pstLib);

CPE_DR_API LPDRMETALIB dr_lib_attach(void const * p, size_t size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

