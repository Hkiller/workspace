#ifndef UI_CACHE_WALKER_H
#define UI_CACHE_WALKER_H
#include "ui_cache_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_cache_worker_t ui_cache_worker_create(ui_cache_manager_t cache_mgr);
void ui_cache_worker_free(ui_cache_worker_t worker);

#ifdef __cplusplus
}
#endif

#endif

