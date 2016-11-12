#ifndef CPE_DR_METALIB_VALIDATE_H
#define CPE_DR_METALIB_VALIDATE_H
#include "cpe/utils/stream.h"
#include "cpe/dr/dr_types.h"
#include "cpe/dr/dr_external.h"

#ifdef __cplusplus
extern "C" {
#endif

int dr_metalib_validate_align(error_monitor_t em, LPDRMETALIB metaLib);

#ifdef __cplusplus
}
#endif


#endif
