#ifndef GD_DR_REF_H
#define GD_DR_REF_H
#include "dr_store_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dr_ref_t dr_ref_create(dr_store_manage_t, const char * name);
void dr_ref_free(dr_ref_t dr_ref);

const char * dr_ref_lib_name(dr_ref_t dr_ref);
LPDRMETALIB dr_ref_lib(dr_ref_t dr_ref);

#ifdef __cplusplus
}
#endif

#endif
