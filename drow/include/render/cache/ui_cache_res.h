#ifndef UI_CACHE_RES_H
#define UI_CACHE_RES_H
#include "ui_cache_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_cache_res_it {
    ui_cache_res_t (*next)(struct ui_cache_res_it * it);
    char m_data[64];
};

ui_cache_res_t ui_cache_res_create(ui_cache_manager_t mgr, ui_cache_res_type_t res_type);
void ui_cache_res_free(ui_cache_res_t res);

int ui_cache_res_set_path(ui_cache_res_t res, const char * src_path);
const char * ui_cache_res_path(ui_cache_res_t res);
ui_cache_res_t ui_cache_res_find_by_path(ui_cache_manager_t mgr, const char * path);

ui_cache_res_t ui_cache_res_create_by_path(ui_cache_manager_t mgr, const char * path);    
ui_cache_res_t ui_cache_res_check_create_by_path(ui_cache_manager_t mgr, const char * path);

ui_cache_res_type_t ui_cache_res_type(ui_cache_res_t res);
const char * ui_cache_res_load_state_to_str(ui_cache_res_load_state_t load_state);
const char * ui_cache_res_type_to_str(ui_cache_res_type_t res_type);
    
uint32_t ui_cache_res_ref_count(ui_cache_res_t res);
void ui_cache_res_ref_inc(ui_cache_res_t res);
void ui_cache_res_ref_dec(ui_cache_res_t res);
    
/*load*/
ui_cache_res_load_state_t ui_cache_res_load_state(ui_cache_res_t res);
int ui_cache_res_load_from_buf(ui_cache_res_t res, ui_cache_pixel_buf_t buf);
int ui_cache_res_load_sync(ui_cache_res_t res, const char * root);
int ui_cache_res_load_async(ui_cache_res_t res, const char * root);
ui_cache_res_load_result_t ui_cache_res_load_result(ui_cache_res_t res);
void ui_cache_res_unload(ui_cache_res_t res, uint8_t is_external_unload);
void ui_cache_res_unload_data(ui_cache_res_t res);
uint8_t ui_cache_res_is_loading(ui_cache_res_t res);

/*group*/
uint8_t ui_cache_res_in_group(ui_cache_res_t res, ui_cache_group_t group);
ui_cache_res_using_state_t ui_cache_res_using_state(ui_cache_res_t res);

/*deleting*/    
void ui_cache_res_tag_delete(ui_cache_res_t res);
void ui_cache_res_free_deleting(ui_cache_manager_t mgr);
    
/*iterator*/
#define ui_cache_res_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

