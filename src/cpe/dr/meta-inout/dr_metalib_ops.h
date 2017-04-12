#ifndef CPE_DR_METAINOUT_INTERNALS_INIT_H
#define CPE_DR_METAINOUT_INTERNALS_INIT_H
#include "cpe/utils/error.h"
#include "cpe/dr/dr_metalib_build.h"
#include "../dr_internal_types.h"
#include "dr_inbuild_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int dr_lib_init(LPDRMETALIB pstLib, const LPDRLIBPARAM pstParam);
void * dr_lib_alloc_in_strbuf(LPDRMETALIB metaLib, size_t size, int * usedCount, error_monitor_t em);

LPDRMACRO dr_lib_add_macro(LPDRMETALIB metaLib, LPDRMACRO macro, error_monitor_t em);

LPDRMETA dr_lib_add_meta(LPDRMETALIB metaLib, LPDRMETA meta, error_monitor_t em);

LPDRMETAENTRY dr_meta_add_entry( LPDRMETA meta, LPDRMETAENTRY entry, error_monitor_t em);
void dr_meta_add_key(LPDRMETA meta, const char * entry_name, error_monitor_t em);

struct dr_index_info * dr_meta_add_index(LPDRMETA meta, struct dr_index_info * info, error_monitor_t em);

void dr_index_add_entry(struct dr_index_info * index, const char * entry_name, error_monitor_t em);

void dr_meta_do_complete(LPDRMETA meta, error_monitor_t em);

int dr_lib_addr_to_pos(LPDRMETALIB metaLib, const void * addr);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif
