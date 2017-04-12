#ifndef UI_RUNTIME_RENDER_OBJ_META_H
#define UI_RUNTIME_RENDER_OBJ_META_H
#include "render/model/ui_object_ref.h"
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_render_obj_meta_it {
    ui_runtime_render_obj_meta_t (*next)(struct ui_runtime_render_obj_meta_it * it);
    char m_data[64];
};

typedef int (*ui_runtime_render_obj_init_fun_t) (void * ctx, ui_runtime_render_obj_t obj);
typedef void (*ui_runtime_render_obj_free_fun_t) (void * ctx, ui_runtime_render_obj_t obj);

typedef void (*ui_runtime_render_obj_bounding_fun_t) (void * ctx, ui_runtime_render_obj_t obj, ui_rect_t bounding);
    
typedef int (*ui_runtime_render_obj_set_fun_t) (void * ctx, ui_runtime_render_obj_t obj, UI_OBJECT_URL const * obj_url);
    
typedef int (*ui_runtime_render_obj_render_fun_t) (
    void * ctx, ui_runtime_render_obj_t obj, ui_runtime_render_t r, ui_rect_t clip_rect, ui_runtime_render_second_color_t second_color, ui_transform_t t);
    
typedef int (*ui_runtime_render_obj_setup_fun_t) (void * ctx, ui_runtime_render_obj_t obj, char * arg_buf_will_change);
    
typedef void (*ui_runtime_render_obj_update_fun_t) (void * ctx, ui_runtime_render_obj_t obj, float delta);

typedef uint8_t (*ui_runtime_render_obj_is_playing_fun_t) (void * ctx, ui_runtime_render_obj_t obj);
typedef int (*ui_runtime_render_obj_resize_fun_t) (void * ctx, ui_runtime_render_obj_t obj, ui_vector_2_t size);
    
ui_runtime_render_obj_meta_t
ui_runtime_render_obj_meta_create(
    ui_runtime_module_t module,
    const char * ui_obj_type_name,
    uint8_t ui_obj_type_id,
    size_t data_capacity,
    void * ctx,
    ui_runtime_render_obj_init_fun_t init_fun,
    ui_runtime_render_obj_set_fun_t set_fun,
    ui_runtime_render_obj_setup_fun_t setup_fun,
    ui_runtime_render_obj_update_fun_t update_fun,
    ui_runtime_render_obj_free_fun_t free_fun,
    ui_runtime_render_obj_render_fun_t render_fun,
    ui_runtime_render_obj_is_playing_fun_t is_playing_fun,
    ui_runtime_render_obj_bounding_fun_t bounding_fun,
    ui_runtime_render_obj_resize_fun_t resize_fun);

void ui_runtime_render_obj_meta_free(
    ui_runtime_render_obj_meta_t obj_meta);

ui_runtime_render_obj_meta_t
ui_runtime_render_obj_meta_find_by_id(ui_runtime_module_t module, uint8_t ui_obj_type);

ui_runtime_render_obj_meta_t
ui_runtime_render_obj_meta_find_by_name(ui_runtime_module_t module, const char * type_name);

const char * ui_runtime_render_obj_meta_type_name(ui_runtime_render_obj_meta_t meta);
uint32_t ui_runtime_render_obj_meta_obj_count(ui_runtime_render_obj_meta_t meta);
void * ui_runtime_render_obj_meta_ctx(ui_runtime_render_obj_meta_t meta);

void ui_runtime_render_obj_metas(ui_runtime_module_t module, ui_runtime_render_obj_meta_it_t meta_it);

#define ui_runtime_render_obj_meta_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
