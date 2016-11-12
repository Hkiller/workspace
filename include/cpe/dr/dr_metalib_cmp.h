#ifndef CPE_DR_METALIB_CMP_H
#define CPE_DR_METALIB_CMP_H
#include <stdio.h>
#include "cpe/dr/dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int dr_meta_compatible(LPDRMETA l, LPDRMETA r);
int dr_metalib_compatible_part(LPDRMETALIB full, LPDRMETALIB part);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
