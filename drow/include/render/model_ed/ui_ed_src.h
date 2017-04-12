#ifndef UI_MODEL_ED_SRC_H
#define UI_MODEL_ED_SRC_H
#include "cpe/utils/stream.h"
#include "cpe/utils/buffer.h"
#include "ui_ed_types.h"
#include "protocol/render/model_ed/ui_ed_src.h"

#ifdef __cplusplus
extern "C" {
#endif

/*load and unload*/
#define ui_ed_src_is_loaded(src) (ui_ed_src_load_state(src) == ui_data_src_state_loaded)
ui_data_src_load_state_t ui_ed_src_load_state(ui_ed_src_t ed_src);
int ui_ed_src_load(ui_ed_src_t ed_src, error_monitor_t em);
void ui_ed_src_unload(ui_ed_src_t ed_src);

/*find*/
ui_ed_src_t ui_ed_src_find_by_path(ui_ed_mgr_t ed_mgr, const char * path, ui_data_src_type_t type);
ui_ed_src_t ui_ed_src_find_by_id(ui_ed_mgr_t ed_mgr, uint32_t id);
ui_ed_src_t ui_ed_src_find_by_data(ui_ed_mgr_t ed_mgr, ui_data_src_t data_src);

/*operations */
ui_ed_obj_t ui_ed_src_root_obj(ui_ed_src_t ed_src);
void ui_ed_src_path_print(write_stream_t s, ui_ed_src_t ed_src);
const char * ui_ed_src_path_dump(mem_buffer_t buffer, ui_ed_src_t ed_src);
ui_data_src_type_t ui_ed_src_type(ui_ed_src_t src);
uint32_t ui_ed_src_id(ui_ed_src_t src);
ui_data_src_t ui_ed_src_data(ui_ed_src_t src);
ui_ed_src_state_t ui_ed_src_state(ui_ed_src_t ed_src);
void * ui_ed_src_product(ui_ed_src_t ed_src);

/*create or remove src*/
ui_ed_src_t ui_ed_src_check_create(ui_ed_mgr_t ed_mgr, const char * path, ui_data_src_type_t type);
ui_ed_src_t ui_ed_src_new(ui_ed_mgr_t ed_mgr, const char * path, ui_data_src_type_t type);
int ui_ed_src_init(ui_ed_src_t ed_src);
int ui_ed_src_save(ui_ed_src_t src, const char * root, error_monitor_t em);
void ui_ed_src_delete(ui_ed_src_t ed_src);
void ui_ed_src_touch(ui_ed_src_t ed_src);

void ui_ed_src_strings_clear(ui_ed_src_t ed_src);
uint32_t ui_ed_src_msg_alloc(ui_ed_src_t ed_src, const char * msg);
void ui_ed_src_msg_remove(ui_ed_src_t ed_src, uint32_t msg_id);
uint32_t ui_ed_src_msg_update(ui_ed_src_t ed_src, uint32_t id, const char * msg);
const char * ui_ed_src_msg(ui_ed_src_t ed_src, uint32_t id);
    
#ifdef __cplusplus
}
#endif

#endif

