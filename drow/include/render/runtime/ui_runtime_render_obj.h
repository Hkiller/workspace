#ifndef UI_RUNTIME_RENDER_OBJ_H
#define UI_RUNTIME_RENDER_OBJ_H
#include "render/model/ui_object_ref.h"
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_runtime_render_obj_t
ui_runtime_render_obj_create_by_url(ui_runtime_module_t module, UI_OBJECT_URL const * obj_url, const char * name);
ui_runtime_render_obj_t
ui_runtime_render_obj_create_by_res(ui_runtime_module_t module, const char * res, const char * name);
ui_runtime_render_obj_t
ui_runtime_render_obj_create_by_type(ui_runtime_module_t module, const char * name, const char * type_name);
ui_runtime_render_obj_t
ui_runtime_render_obj_create(ui_runtime_module_t module, const char * name, ui_runtime_render_obj_meta_t obj_meta);

void ui_runtime_render_obj_free(ui_runtime_render_obj_t obj);

ui_runtime_module_t ui_runtime_render_obj_module(ui_runtime_render_obj_t obj);
const char * ui_runtime_render_obj_name(ui_runtime_render_obj_t obj);

ui_runtime_render_obj_t
ui_runtime_render_obj_find(ui_runtime_module_t module, const char * name);

int ui_runtime_render_obj_get_bounding(ui_runtime_render_obj_t obj, ui_rect_t bounding);
    
void ui_runtime_render_obj_update(ui_runtime_render_obj_t obj, float delta);

int ui_runtime_render_obj_set_url(ui_runtime_render_obj_t obj, UI_OBJECT_URL const * obj_url);
int ui_runtime_render_obj_setup(ui_runtime_render_obj_t obj, char * arg_buf_will_change);
    
ui_data_src_t ui_runtime_render_obj_src(ui_runtime_render_obj_t obj);
void ui_runtime_render_obj_set_src(ui_runtime_render_obj_t obj, ui_data_src_t src);

uint8_t ui_runtime_render_obj_type_id(ui_runtime_render_obj_t obj);
const char * ui_runtime_render_obj_type_name(ui_runtime_render_obj_t obj);
ui_runtime_render_obj_meta_t ui_runtime_render_obj_meta(ui_runtime_render_obj_t obj);
void * ui_runtime_render_obj_data(ui_runtime_render_obj_t obj);
ui_runtime_render_obj_t ui_runtime_render_obj_from_data(void * data);

uint8_t ui_runtime_render_obj_is_suspend(ui_runtime_render_obj_t obj);
void ui_runtime_render_obj_set_suspend(ui_runtime_render_obj_t obj, uint8_t suspend);

uint8_t ui_runtime_render_obj_support_update(ui_runtime_render_obj_t obj);

uint8_t ui_runtime_render_obj_is_playing(ui_runtime_render_obj_t obj);

uint8_t ui_runtime_render_obj_auto_release(ui_runtime_render_obj_t obj);
void ui_runtime_render_obj_set_auto_release(ui_runtime_render_obj_t obj, uint8_t auto_release);

uint8_t ui_runtime_render_obj_keep_update(ui_runtime_render_obj_t obj);
void ui_runtime_render_obj_set_keep_update(ui_runtime_render_obj_t obj, uint8_t keep_update);

float ui_runtime_render_obj_time_scale(ui_runtime_render_obj_t obj);
void ui_runtime_render_obj_set_time_scale(ui_runtime_render_obj_t obj, float time_scale);

ui_vector_2_t ui_runtime_render_obj_size(ui_runtime_render_obj_t obj);
void ui_runtime_render_obj_set_size(ui_runtime_render_obj_t obj, ui_vector_2_t size);

uint16_t ui_runtime_render_obj_ref_count(ui_runtime_render_obj_t obj);
ui_runtime_render_obj_ref_t ui_runtime_render_obj_updator(ui_runtime_render_obj_t obj);

void ui_runtime_render_obj_set_evt_processor(ui_runtime_render_obj_t obj, ui_runtime_render_obj_evt_fun_t fun, void * ctx);
    
void ui_runtime_render_obj_send_event(ui_runtime_render_obj_t obj, const char * evt);

ui_transform_t ui_runtime_render_obj_transform(ui_runtime_render_obj_t obj);

uint8_t ui_runtime_render_obj_touch_is_support(ui_runtime_render_obj_t obj);
void ui_runtime_render_obj_touch_set_processor(
    ui_runtime_render_obj_t obj, ui_runtime_touch_process_fun_t process_fun, void * process_ctx);    

int ui_runtime_render_obj_add_child(ui_runtime_render_obj_t obj, ui_runtime_render_obj_ref_t child_obj_ref, void const * tag, uint8_t auto_render);
ui_runtime_render_obj_ref_t ui_runtime_render_obj_find_child(ui_runtime_render_obj_t obj, void const * tag);
void ui_runtime_render_obj_clear_childs(ui_runtime_render_obj_t obj);
    
int ui_runtime_render_obj_render(ui_runtime_render_obj_t obj, ui_runtime_render_t ctx, ui_rect_t clip_rect, ui_transform_t t);

#ifdef __cplusplus
}
#endif

#endif
