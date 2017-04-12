#ifndef UI_MODEL_DATA_MGR_H
#define UI_MODEL_DATA_MGR_H
#include "gd/app/app_types.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_data_mgr_t
ui_data_mgr_create(
    gd_app_context_t app, mem_allocrator_t alloc, const char * name, error_monitor_t em,
    ui_cache_manager_t cache_mgr);
void ui_data_mgr_free(ui_data_mgr_t mgr);

ui_data_mgr_t ui_data_mgr_find(gd_app_context_t app, cpe_hash_string_t name);
ui_data_mgr_t ui_data_mgr_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t ui_data_mgr_app(ui_data_mgr_t mgr);
const char * ui_data_mgr_name(ui_data_mgr_t mgr);

ui_cache_manager_t ui_data_mgr_cache_mgr(ui_data_mgr_t mgr);

ui_data_src_t ui_data_mgr_src_root(ui_data_mgr_t mgr);
int ui_data_mgr_set_root(ui_data_mgr_t mgr, const char * root);

void ui_data_mgr_set_loader(ui_data_mgr_t mgr, ui_data_src_type_t type, product_load_fun_t loader, void * ctx);
void ui_data_mgr_set_saver(ui_data_mgr_t mgr, ui_data_src_type_t type, product_save_fun_t saver, product_remove_fun_t remover, void * ctx);

int ui_data_mgr_register_type(
    ui_data_mgr_t mgr, ui_data_src_type_t type,
    ui_data_product_create_fun_t product_create, void * product_create_ctx,
    ui_data_product_free_fun_t product_free, void * product_free_ctx,
    product_using_src_update_using_fun_t update_using);

int ui_data_mgr_unregister_type(ui_data_mgr_t mgr, ui_data_src_type_t type);

int ui_data_mgr_save(ui_data_mgr_t mgr, const char * dir);
int ui_data_mgr_save_by_type(ui_data_mgr_t mgr, const char * dir, ui_data_src_type_t type);

LPDRMETA ui_data_mgr_meta_object_url(ui_data_mgr_t mgr);
LPDRMETA ui_data_mgr_meta_control_object_url(ui_data_mgr_t mgr);

#ifdef __cplusplus
}
#endif

#endif 
