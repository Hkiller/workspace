#ifndef UI_RUNTIME_RENDER_OBJ_REF_H
#define UI_RUNTIME_RENDER_OBJ_REF_H
#include "render/model/ui_object_ref.h"
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_runtime_render_obj_ref_t ui_runtime_render_obj_ref_create_by_res(ui_runtime_module_t module, const char * res, char ** left_args);
ui_runtime_render_obj_ref_t ui_runtime_render_obj_ref_create_by_obj(ui_runtime_render_obj_t obj);
ui_runtime_render_obj_ref_t ui_runtime_render_obj_ref_create_by_obj_url(ui_runtime_module_t module, UI_OBJECT_URL const * obj_url, const char * name);
ui_runtime_render_obj_ref_t ui_runtime_render_obj_ref_create_by_obj_name(ui_runtime_module_t module, const char * name);
ui_runtime_render_obj_ref_t ui_runtime_render_obj_ref_clone(ui_runtime_render_obj_ref_t obj_ref);
    
void ui_runtime_render_obj_ref_free(ui_runtime_render_obj_ref_t obj_ref);

ui_runtime_module_t ui_runtime_render_obj_ref_module(ui_runtime_render_obj_ref_t obj_ref);
ui_runtime_render_obj_t ui_runtime_render_obj_ref_obj(ui_runtime_render_obj_ref_t obj_ref);

const char * ui_runtime_render_obj_ref_type_name(ui_runtime_render_obj_ref_t obj_ref);
    
int ui_runtime_render_obj_ref_setup(ui_runtime_render_obj_ref_t obj_ref, char * arg_buf_will_change);
    
ui_runtime_render_second_color_t ui_runtime_render_obj_ref_second_color(ui_runtime_render_obj_ref_t obj_ref);
void ui_runtime_render_obj_ref_set_second_color(ui_runtime_render_obj_ref_t obj_ref, ui_runtime_render_second_color_t second_color);

uint8_t ui_runtime_render_obj_ref_is_hide(ui_runtime_render_obj_ref_t obj_ref);
void ui_runtime_render_obj_ref_set_hide(ui_runtime_render_obj_ref_t obj_ref, uint8_t hide);

ui_transform_t ui_runtime_render_obj_ref_transform(ui_runtime_render_obj_ref_t obj_ref);
void ui_runtime_render_obj_ref_set_transform(ui_runtime_render_obj_ref_t obj_ref, ui_transform_t trans);    

void ui_runtime_render_obj_ref_transform_set_to_obj(ui_runtime_render_obj_ref_t obj_ref, ui_transform_t input_t);

uint8_t ui_runtime_render_obj_ref_is_updator(ui_runtime_render_obj_ref_t obj_ref);    
void ui_runtime_render_obj_ref_set_is_updator(ui_runtime_render_obj_ref_t obj_ref, uint8_t is_updator);
    
int ui_runtime_render_obj_ref_render(ui_runtime_render_obj_ref_t obj_ref, ui_runtime_render_t ctx, ui_rect_t clip_rect, ui_transform_t t);
void ui_runtime_render_obj_ref_update(ui_runtime_render_obj_ref_t obj_ref, float delta);

void ui_runtime_render_obj_ref_set_args(ui_runtime_render_obj_ref_t obj_ref, const char * arg);

void ui_runtime_render_obj_ref_set_evt_processor(ui_runtime_render_obj_ref_t obj_ref, ui_runtime_render_obj_evt_fun_t fun, void * ctx);

uint8_t ui_runtime_render_obj_ref_touch_is_support(ui_runtime_render_obj_ref_t obj_ref);
int ui_runtime_render_obj_ref_touch_dispatch(
    ui_runtime_render_obj_ref_t obj_ref, uint32_t track_id, ui_runtime_touch_event_t evt, ui_vector_2_t screen_pt, ui_vector_2_t logic_pt);
    
#ifdef __cplusplus
}
#endif

#endif
