#ifndef PLUGIN_LAYOUT_FONT_META_H
#define PLUGIN_LAYOUT_FONT_META_H
#include "plugin_layout_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*plugin_layout_font_meta_init_fun_t)(void * ctx, plugin_layout_font_meta_t meta);
typedef void (*plugin_layout_font_meta_fini_fun_t)(void * ctx, plugin_layout_font_meta_t meta);
typedef void (*plugin_layout_font_meta_on_cache_clear_fun_t)(void * ctx, plugin_layout_font_meta_t meta);    
typedef int (*plugin_layout_font_face_init_fun_t)(void * ctx, plugin_layout_font_face_t face);
typedef void (*plugin_layout_font_face_fini_fun_t)(void * ctx, plugin_layout_font_face_t face);
typedef int (*plugin_layout_font_element_init_fun_t)(void * ctx, plugin_layout_font_element_t element);
typedef void (*plugin_layout_font_element_fini_fun_t)(void * ctx, plugin_layout_font_element_t element);
typedef void (*plugin_layout_font_element_render_fun_t)(
    void * ctx, plugin_layout_font_element_t element, plugin_layout_font_draw_t font_draw, ui_rect_t target_rect,
    ui_runtime_render_t render, ui_runtime_render_cmd_t * batch_cmd,
    ui_rect_t clip_rect, ui_runtime_render_second_color_t second_color, ui_transform_t transform);
    
typedef int (*plugin_layout_font_meta_basic_layout_fun_t)(
    void * ctx, plugin_layout_font_face_t face, plugin_layout_render_t render,
    plugin_layout_font_draw_t font_draw,
    uint32_t const * text, size_t text_len, plugin_layout_render_group_t group);

plugin_layout_font_meta_t
plugin_layout_font_meta_create(
    plugin_layout_module_t module,
    plugin_layout_font_category_t category,
    const char * name,
    void * ctx,
    uint32_t meta_capacity,
    plugin_layout_font_meta_init_fun_t init_meta,
    plugin_layout_font_meta_fini_fun_t fini_meta,
    plugin_layout_font_meta_on_cache_clear_fun_t on_cache_clear,
    uint32_t face_capacity,
    plugin_layout_font_face_init_fun_t init_face,
    plugin_layout_font_face_fini_fun_t fini_face,
    uint32_t element_capacity,
    plugin_layout_font_element_init_fun_t init_element,
    plugin_layout_font_element_fini_fun_t fini_element,
    plugin_layout_font_element_render_fun_t render_element,
    /*layout*/
    plugin_layout_font_meta_basic_layout_fun_t basic_layout);
    
void plugin_layout_font_meta_free(plugin_layout_font_meta_t meta);
void * plugin_layout_font_meta_data(plugin_layout_font_meta_t meta);

plugin_layout_font_meta_t
plugin_layout_font_meta_find_by_category(
    plugin_layout_module_t module, plugin_layout_font_category_t category);

plugin_layout_font_meta_t
plugin_layout_font_meta_find_by_name(
    plugin_layout_module_t module, const char * name);
    
#ifdef __cplusplus
}
#endif

#endif
