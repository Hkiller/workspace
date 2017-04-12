#ifndef UI_MODEL_ED_MGR_H
#define UI_MODEL_ED_MGR_H
#include "gd/app/app_types.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "ui_ed_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_ed_mgr_t
ui_ed_mgr_create(
    gd_app_context_t app, ui_data_mgr_t data_mgr, 
    mem_allocrator_t alloc, const char * name, error_monitor_t em);
void ui_ed_mgr_free(ui_ed_mgr_t mgr);

ui_ed_mgr_t ui_ed_mgr_find(gd_app_context_t app, cpe_hash_string_t name);
ui_ed_mgr_t ui_ed_mgr_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t ui_ed_mgr_app(ui_ed_mgr_t mgr);
const char * ui_ed_mgr_name(ui_ed_mgr_t mgr);

ui_data_mgr_t ui_ed_mgr_data_mgr(ui_ed_mgr_t ed_mgr);

void ui_ed_mgr_free_src_by_type(ui_ed_mgr_t ed_mgr, ui_data_src_type_t src_type);
void ui_ed_mgr_free_obj_by_type(ui_ed_mgr_t ed_mgr, ui_ed_obj_type_t obj_type);

int ui_ed_mgr_register_src_type(ui_ed_mgr_t mgr, ui_data_src_type_t type, ui_ed_src_load_fun_t load_fun);
int ui_ed_mgr_unregister_src_type(ui_ed_mgr_t mgr, ui_data_src_type_t type);

int ui_ed_mgr_save(ui_ed_mgr_t mgr, const char * root, error_monitor_t em);

int ui_ed_mgr_register_obj_type(
    ui_ed_mgr_t mgr, ui_data_src_type_t src_type, ui_ed_obj_type_t obj_type, LPDRMETA obj_meta,
    ui_ed_obj_delete_fun_t delete_fun,
    const char * id_entry_name, ui_ed_obj_set_id_fun_t id_set_fun);

int ui_ed_mgr_register_obj_child(
    ui_ed_mgr_t mgr, ui_ed_obj_type_t parent_obj_type,
    ui_ed_obj_type_t child_obj_type, ui_ed_obj_create_fun_t create_fun);

int ui_ed_mgr_unregister_obj_type(ui_ed_mgr_t mgr, ui_ed_obj_type_t obj_type);

#ifdef __cplusplus
}
#endif

#endif 
