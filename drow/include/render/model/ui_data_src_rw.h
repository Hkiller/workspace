#ifndef UI_MODEL_DATA_SRC_RW_H
#define UI_MODEL_DATA_SRC_RW_H
#include "cpe/vfs/vfs_types.h"
#include "ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*ui_data_src_do_save_fun_t)(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em);
typedef int (*ui_data_src_do_load_fun_t)(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em);

int ui_data_src_save_to_file(
    ui_data_src_t src, const char * root, const char * postfix,
    ui_data_src_do_save_fun_t save_fun, void * save_fun_ctx,
    error_monitor_t em);

int ui_data_src_remove_file(
    ui_data_src_t src, const char * root, const char * postfix,
    error_monitor_t em);

int ui_data_src_load_from_file(
    ui_data_src_t src, const char * postfix,
    ui_data_src_do_load_fun_t load_fun, void * load_fun_ctx,
    error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
