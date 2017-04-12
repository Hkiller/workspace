#ifndef UI_MODEL_BIN_H
#define UI_MODEL_BIN_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "ui_bin_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_bin_loader_t ui_bin_loader_create(
    gd_app_context_t app, mem_allocrator_t alloc, const char * name, error_monitor_t em);

void ui_bin_loader_free(ui_bin_loader_t loader);

ui_bin_loader_t ui_bin_loader_find(gd_app_context_t app, cpe_hash_string_t name);
ui_bin_loader_t ui_bin_loader_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t ui_bin_loader_app(ui_bin_loader_t loader);
const char * ui_bin_loader_name(ui_bin_loader_t loader);

const char * ui_bin_loader_root(ui_bin_loader_t loader);
int ui_bin_loader_set_root(ui_bin_loader_t loader, const char * root);

//int ui_data_bin_loader_load(ui_bin_loader_t loader, ui_data_mgr_t mgr, int load_product);

int ui_data_bin_loader_set_load_to_data_mgr(ui_bin_loader_t loader, ui_data_mgr_t mgr);
int ui_data_bin_loader_set_save_to_data_mgr(ui_bin_loader_t loader, ui_data_mgr_t mgr);

#ifdef __cplusplus
}
#endif

#endif
