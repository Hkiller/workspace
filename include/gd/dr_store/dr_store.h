#ifndef GD_DR_STORE_H
#define GD_DR_STORE_H
#include "dr_store_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dr_store_t dr_store_create(dr_store_manage_t, const char * name);
void dr_store_free(dr_store_t dr_store);

dr_store_t
dr_store_find(dr_store_manage_t mgr, const char * name);

LPDRMETALIB dr_store_lib(dr_store_t dr_store);

dr_store_t
dr_store_find_or_create(dr_store_manage_t mgr, const char * name);

int dr_store_set_lib(dr_store_t dr_store, LPDRMETALIB lib, dr_lib_free_fun_t free_fun, void * free_ctx);
void dr_store_reset_lib(dr_store_t dr_store, LPDRMETALIB lib, dr_lib_free_fun_t free_fun, void * free_ctx);

int dr_store_add_lib(
    dr_store_manage_t mgr, const char * name,
    LPDRMETALIB lib, dr_lib_free_fun_t free_fun, void * free_ctx);

#ifdef __cplusplus
}
#endif

#endif
