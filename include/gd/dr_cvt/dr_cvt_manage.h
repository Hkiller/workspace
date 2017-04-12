#ifndef GD_DR_CVT_MANAGE_H
#define GD_DR_CVT_MANAGE_H
#include "dr_cvt_types.h"

#ifdef __cplusplus
extern "C" {
#endif

dr_cvt_manage_t
dr_cvt_manage_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em);

void dr_cvt_manage_free(dr_cvt_manage_t mgr);

dr_cvt_manage_t
dr_cvt_manage_find(gd_app_context_t app, cpe_hash_string_t name);

dr_cvt_manage_t
dr_cvt_manage_find_nc(gd_app_context_t app, const char * name);

dr_cvt_manage_t
dr_cvt_manage_default(gd_app_context_t app);

gd_app_context_t dr_cvt_manage_app(dr_cvt_manage_t mgr);
const char * dr_cvt_manage_name(dr_cvt_manage_t mgr);
cpe_hash_string_t dr_cvt_manage_name_hs(dr_cvt_manage_t mgr);

int dr_cvt_type_create_ex(dr_cvt_manage_t mgr, const char * name, dr_cvt_fun_t encode, dr_cvt_fun_t decode, void * ctx);
void dr_cvt_type_free_ex(dr_cvt_manage_t mgr, const char * name);

int dr_cvt_type_create(gd_app_context_t app, const char * name, dr_cvt_fun_t encode, dr_cvt_fun_t decode, void * ctx);
void dr_cvt_type_free(gd_app_context_t app, const char * name);


#ifdef __cplusplus
}
#endif

#endif
