#ifndef UI_MODEL_DATA_SRC_H
#define UI_MODEL_DATA_SRC_H
#include "cpe/utils/stream.h"
#include "cpe/utils/buffer.h"
#include "protocol/render/model/ui_object_ref.h"
#include "ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_data_src_it {
    ui_data_src_t (*next)(struct ui_data_src_it * it);
    char m_data[64];
};

/*src create and free*/
ui_data_src_t ui_data_src_create_file(ui_data_mgr_t mgr, ui_data_src_type_t type, const char * full_file);
ui_data_src_t ui_data_src_create_relative(ui_data_mgr_t mgr, ui_data_src_type_t type, const char * path);
ui_data_src_t ui_data_src_create_child(ui_data_src_t parent, ui_data_src_type_t type, const char * name);
void ui_data_src_free(ui_data_src_t src);

/*src mgr*/
uint32_t ui_data_src_id(ui_data_src_t src);
int ui_data_src_set_id(ui_data_src_t src, uint32_t id);
ui_data_mgr_t ui_data_src_mgr(ui_data_src_t src);

ui_data_src_type_t ui_data_src_type(ui_data_src_t src);
const char * ui_data_src_data(ui_data_src_t src);
const char * ui_data_src_data_with_language(ui_data_src_t src, char * buf, size_t buf_size);
int ui_data_src_set_data(ui_data_src_t src, const char * data);
void ui_data_src_data_print(write_stream_t s, ui_data_src_t src);

ui_data_src_t ui_data_src_find_by_id(ui_data_mgr_t mgr, uint32_t id);
ui_data_src_t ui_data_src_find_by_path(ui_data_mgr_t mgr, const char * path, ui_data_src_type_t type);
ui_data_src_t ui_data_src_find_by_url(ui_data_mgr_t mgr, UI_OBJECT_URL const * url);
ui_data_src_t ui_data_src_find_by_res(ui_data_mgr_t mgr, const char * res);

/*src and language*/
ui_data_language_t ui_data_src_language(ui_data_src_t src);
ui_data_src_t ui_data_src_language_src(ui_data_src_t src);
ui_data_src_t ui_data_src_base_src(ui_data_src_t src);
const char * ui_data_src_language_name(ui_data_src_t src);
    
/*src and product*/
#define ui_data_src_is_loaded(src) (ui_data_src_load_state(src) == ui_data_src_state_loaded)

ui_data_src_load_state_t ui_data_src_load_state(ui_data_src_t src);
void * ui_data_src_product(ui_data_src_t src);
void ui_data_src_set_product(ui_data_src_t src, void * product);

int ui_data_src_load(ui_data_src_t src, error_monitor_t em);
int ui_data_src_init(ui_data_src_t src);
void ui_data_src_unload(ui_data_src_t src);
int ui_data_src_save(ui_data_src_t src, const char * root, error_monitor_t em);
int ui_data_src_remove(ui_data_src_t src, const char * root, error_monitor_t em);

int ui_data_src_check_load_with_usings(ui_data_src_t src, error_monitor_t em);
    
/*src tree*/
ui_data_src_t ui_data_src_parent(ui_data_src_t src);
void ui_data_src_childs(ui_data_src_it_t it, ui_data_src_t src);
void ui_data_src_all_childs(ui_data_src_it_t it, ui_data_src_t src);
ui_data_src_t ui_data_src_child_find(ui_data_src_t src, const char * name, ui_data_src_type_t type);
ui_data_src_t ui_data_src_child_find_with_language(ui_data_src_t src, const char * name, ui_data_src_type_t type, ui_data_language_t language);
ui_data_src_t ui_data_src_child_find_by_path(ui_data_src_t src, const char * path, ui_data_src_type_t type);
void ui_data_src_path_print_to(write_stream_t s, ui_data_src_t src, ui_data_src_t stop);
void ui_data_src_path_print(write_stream_t s, ui_data_src_t src);
const char * ui_data_src_path_dump(mem_buffer_t buff, ui_data_src_t src);
    
/*src using relation*/
#define ui_data_src_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
int ui_data_src_update_using(ui_data_src_t src);
void ui_data_src_clear_using(ui_data_src_t src);

void ui_data_src_using_srcs(ui_data_src_t src, ui_data_src_it_t src_it);
void ui_data_src_user_srcs(ui_data_src_t src, ui_data_src_it_t src_it);
void ui_data_src_using_ress(ui_data_src_t src, ui_cache_res_it_t res_it);
int ui_data_src_collect_res_from_event(ui_data_src_t user, const char * event_name, const char * args);

int ui_data_src_collect_ress(ui_data_src_t src, ui_cache_group_t res_group);

/*group*/
uint8_t ui_data_src_in_group(ui_data_src_t src, ui_data_src_group_t group);

/*static*/
const char * ui_data_src_type_name(ui_data_src_type_t type);

/*string table*/
ui_data_src_strings_state_t ui_data_src_strings_state(ui_data_src_t src);
    
ui_string_table_t ui_data_src_strings(ui_data_src_t src);
const char * ui_data_src_msg(ui_data_src_t src, uint32_t msg_id);
void ui_data_src_msg_remove(ui_data_src_t src, uint32_t msg_id);
uint32_t ui_data_src_msg_alloc(ui_data_src_t src, const char * str);
uint32_t ui_data_src_msg_update(ui_data_src_t src, uint32_t id, const char * msg);

int ui_data_src_strings_set(ui_data_src_t src, void const * data, size_t data_size);
void ui_data_src_strings_clear(ui_data_src_t src);
    
ui_string_table_builder_t ui_data_src_strings_builder(ui_data_src_t src);    
ui_string_table_builder_t ui_data_src_strings_build_begin(ui_data_src_t src);
int ui_data_src_strings_build_complete(ui_data_src_t src);    

#ifdef __cplusplus
}
#endif

#endif
