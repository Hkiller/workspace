#ifndef UI_CACHE_GROUP_H
#define UI_CACHE_GROUP_H
#include "ui_cache_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_cache_group_t ui_cache_group_create(ui_cache_manager_t mgr);
void ui_cache_group_free(ui_cache_group_t group);

ui_cache_manager_t ui_cache_group_mgr(ui_cache_group_t group);
ui_cache_res_using_state_t ui_cache_group_using_state(ui_cache_group_t group);
void ui_cache_group_set_using_state(ui_cache_group_t group, ui_cache_res_using_state_t using_state);

int ui_cache_group_add_res(ui_cache_group_t group, ui_cache_res_t res);
int ui_cache_group_add_res_by_path(ui_cache_group_t group, const char * res_path);
int ui_cache_group_add_res_by_cache_group(ui_cache_group_t group, ui_cache_group_t from_group);

void ui_cache_group_remove_res(ui_cache_group_t group, ui_cache_res_t res);
void ui_cache_group_clear(ui_cache_group_t group);

ui_cache_res_t ui_cache_group_first(ui_cache_group_t group);
    
void ui_cache_group_using_resources(ui_cache_res_it_t it, ui_cache_group_t group);
    
void ui_cache_group_load_all_async(ui_cache_group_t group);
void ui_cache_group_load_all_sync(ui_cache_group_t group);
    
#ifdef __cplusplus
}
#endif

#endif

