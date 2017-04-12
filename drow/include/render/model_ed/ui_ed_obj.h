#ifndef UI_MODEL_ED_OBJ_H
#define UI_MODEL_ED_OBJ_H
#include "cpe/utils/stream.h"
#include "cpe/utils/buffer.h"
#include "ui_ed_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_ed_obj_it {
    ui_ed_obj_t (*next)(struct ui_ed_obj_it * it);
    char m_data[64];
};

ui_ed_obj_t ui_ed_obj_create_i(
    ui_ed_src_t ed_src, ui_ed_obj_t parent, ui_ed_obj_type_t obj_type,
    void * product, void * data, uint16_t data_capacity);

ui_ed_obj_meta_t ui_ed_obj_meta(ui_ed_obj_t obj);
ui_ed_obj_type_t ui_ed_obj_type_id(ui_ed_obj_t obj);

void * ui_ed_obj_product(ui_ed_obj_t obj);
void * ui_ed_obj_data(ui_ed_obj_t obj);
uint16_t ui_ed_obj_data_capacity(ui_ed_obj_t obj);
LPDRMETA ui_ed_obj_data_meta(ui_ed_obj_t obj);

ui_ed_obj_t ui_ed_obj_find_by_id(ui_ed_src_t src, uint32_t id);

ui_ed_obj_t ui_ed_obj_parent(ui_ed_obj_t obj);
void ui_ed_obj_childs(ui_ed_obj_it_t obj_it, ui_ed_obj_t obj);
void ui_ed_obj_childs_by_type(ui_ed_obj_it_t obj_it, ui_ed_obj_t obj, ui_ed_obj_type_t obj_type);
ui_ed_obj_t ui_ed_obj_only_child(ui_ed_obj_t obj);

ui_ed_obj_t ui_ed_obj_first_child_of_type(ui_ed_obj_t obj, ui_ed_obj_type_t obj_type);
ui_ed_obj_t ui_ed_obj_n_child_of_type(ui_ed_obj_t obj, ui_ed_obj_type_t obj_type, uint32_t n);
    
ui_ed_src_t ui_ed_obj_src(ui_ed_obj_t obj);

void ui_ed_obj_all_usings(ui_ed_obj_it_t obj_it, ui_ed_obj_t obj);
void ui_ed_obj_usings(ui_ed_obj_it_t obj_it, ui_ed_obj_t obj, ui_ed_rel_type_t rel_type);
ui_ed_obj_t ui_ed_obj_using_find(ui_ed_obj_t obj, ui_ed_rel_type_t rel_type);

void ui_ed_obj_path_print(write_stream_t s, ui_ed_obj_t obj);
const char * ui_ed_obj_path_dump(mem_buffer_t buffer, ui_ed_obj_t obj);
const char * ui_ed_obj_full_path_dump(mem_buffer_t buffer, ui_ed_obj_t obj);

const char * ui_ed_obj_dump_with_full_path(mem_buffer_t buffer, ui_ed_obj_t obj);

const char * ui_ed_obj_type_name(ui_ed_obj_type_t obj_type);
ui_ed_obj_type_t ui_ed_obj_type_from_name(const char * obj_type_name);
const char * ui_ed_rel_type_name(ui_ed_rel_type_t rel_type);
ui_ed_rel_type_t ui_ed_rel_type_from_name(const char * rel_type_name);

/*obj manip operations*/
ui_ed_obj_t ui_ed_obj_new(ui_ed_obj_t parent, ui_ed_obj_type_t obj_type);
void ui_ed_obj_remove(ui_ed_obj_t ed_obj);
void ui_ed_obj_remove_childs(ui_ed_obj_t ed_obj);
void ui_ed_obj_remove_childs_by_type(ui_ed_obj_t ed_obj, ui_ed_obj_type_t obj_type);
int ui_ed_obj_set_id(ui_ed_obj_t ed_obj, uint32_t id);

/*iterator*/
#define ui_ed_obj_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

