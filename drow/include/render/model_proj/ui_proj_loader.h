#ifndef UI_MODEL_PROJ_H
#define UI_MODEL_PROJ_H
#include "ui_proj_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_proj_loader_t ui_proj_loader_create(
    gd_app_context_t app, mem_allocrator_t alloc, const char * name, error_monitor_t em);

void ui_proj_loader_free(ui_proj_loader_t loader);

ui_proj_loader_t ui_proj_loader_find(gd_app_context_t app, cpe_hash_string_t name);
ui_proj_loader_t ui_proj_loader_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t ui_proj_loader_app(ui_proj_loader_t loader);
const char * ui_proj_loader_name(ui_proj_loader_t loader);

const char * ui_proj_loader_root(ui_proj_loader_t loader);
int ui_proj_loader_set_root(ui_proj_loader_t loader, const char * root);

const char * ui_proj_loader_root(ui_proj_loader_t loader);
int ui_proj_loader_set_root(ui_proj_loader_t loader, const char * root);

int ui_data_proj_loader_set_load_to_data_mgr(ui_proj_loader_t loader, ui_data_mgr_t mgr);
int ui_data_proj_loader_set_save_to_data_mgr(ui_proj_loader_t loader, ui_data_mgr_t mgr);

int ui_data_proj_loader_load(ui_proj_loader_t loader, ui_data_mgr_t mgr, int load_product);

#ifdef __cplusplus
}
#endif

#endif
