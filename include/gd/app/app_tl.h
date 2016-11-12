#ifndef GD_APP_TL_H
#define GD_APP_TL_H
#include "app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

tl_manage_t
app_tl_manage_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc);

void app_tl_manage_free(gd_app_context_t app, tl_manage_t tl_mgr);

tl_manage_t
app_tl_manage_find(gd_app_context_t app, const char * name);


#ifdef __cplusplus
}
#endif

#endif
