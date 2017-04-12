#ifndef DROW_PLUGIN_LAYOUT_ANIMATION_META_H
#define DROW_PLUGIN_LAYOUT_ANIMATION_META_H
#include "plugin_layout_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*plugin_layout_animation_init_fun_t)(plugin_layout_animation_t animation, void * ctx);
typedef void (*plugin_layout_animation_free_fun_t)(plugin_layout_animation_t animation, void * ctx);
typedef void (*plugin_layout_animation_layout_fun_t)(plugin_layout_animation_t animation, void * ctx);
typedef uint8_t (*plugin_layout_animation_update_fun_t)(plugin_layout_animation_t to, void * ctx, float delta_s); /*return need update(bool)*/
typedef void (*plugin_layout_animation_render_fun_t)(
    plugin_layout_animation_t animation, plugin_layout_animation_layer_t layer,
    void * ctx, ui_runtime_render_t context, ui_rect_t clip_rect, ui_runtime_render_second_color_t second_color, ui_transform_t transform);
    
plugin_layout_animation_meta_t
plugin_layout_animation_meta_create(
    plugin_layout_module_t module, const char * type_name, void * ctx,
    size_t anim_capacity,
    plugin_layout_animation_init_fun_t init_fun,
    plugin_layout_animation_free_fun_t fini_fun,
    plugin_layout_animation_layout_fun_t layout_fun,
    plugin_layout_animation_update_fun_t update_fun,
    plugin_layout_animation_render_fun_t render_fun);

void plugin_layout_animation_meta_free(plugin_layout_animation_meta_t meta);

plugin_layout_animation_meta_t
plugin_layout_animation_meta_find(plugin_layout_module_t module, const char * type_name);

#ifdef __cplusplus
}
#endif

#endif

