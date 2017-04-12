#ifndef UI_CACHE_MANAGER_H
#define UI_CACHE_MANAGER_H
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "ui_cache_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_cache_manager_t ui_cache_manager_create(
    gd_app_context_t app, mem_allocrator_t alloc, const char * name, uint32_t task_capacity, error_monitor_t em);
void ui_cache_manager_free(ui_cache_manager_t cache_mgr);

gd_app_context_t ui_cache_manager_app(ui_cache_manager_t cache_mgr);
const char * ui_cache_manager_name(ui_cache_manager_t cache_mgr);

ui_cache_manager_t ui_cache_manager_find(gd_app_context_t app, cpe_hash_string_t name);
ui_cache_manager_t ui_cache_manager_find_nc(gd_app_context_t app, const char * name);

uint16_t ui_cache_manager_loading_count(ui_cache_manager_t mgr);

uint8_t ui_cache_manager_texture_data_buff_keep(ui_cache_manager_t cache_mgr);
void ui_cache_manager_set_texture_data_buff_keep(ui_cache_manager_t cache_mgr, uint8_t keep);

uint8_t ui_cache_manager_texture_scale(ui_cache_manager_t cache_mgr);
void ui_cache_manager_set_texture_scale(ui_cache_manager_t cache_mgr, uint8_t scale);
    
uint32_t ui_cache_manager_runing_task_count(ui_cache_manager_t cache_mgr);

void ui_cache_manager_ress(ui_cache_manager_t mgr, ui_cache_res_it_t it);

#ifdef __cplusplus
}
#endif

#endif

