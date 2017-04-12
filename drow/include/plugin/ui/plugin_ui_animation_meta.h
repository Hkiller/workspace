#ifndef DROW_PLUGIN_UI_ANIMATION_META_H
#define DROW_PLUGIN_UI_ANIMATION_META_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*plugin_ui_animation_init_fun_t)(plugin_ui_animation_t animation, void * ctx);
typedef void (*plugin_ui_animation_free_fun_t)(plugin_ui_animation_t animation, void * ctx);
    
typedef int (*plugin_ui_animation_enter_fun_t)(plugin_ui_animation_t animation, void * ctx);
typedef void (*plugin_ui_animation_exit_fun_t)(plugin_ui_animation_t animation, void * ctx);
typedef uint8_t (*plugin_ui_animation_update_fun_t)(plugin_ui_animation_t to, void * ctx, float delta_s); /*return need update(bool)*/

typedef int (*plugin_ui_animation_control_attach_fun_t)(plugin_ui_animation_control_t animation_control, void * ctx);
typedef void (*plugin_ui_animation_control_detach_fun_t)(plugin_ui_animation_control_t animation_control, void * ctx);

typedef int (*plugin_ui_animation_setup_fun_t)(
    plugin_ui_animation_t animation,
    void * ctx, char * arg_buf_will_change,
    plugin_ui_control_t control, plugin_ui_control_frame_t frame);

plugin_ui_animation_meta_t
plugin_ui_animation_meta_create(
    plugin_ui_module_t module,
    const char * type_name,
    void * ctx,
    /*animation*/
    size_t anim_capacity,
    plugin_ui_animation_init_fun_t init_fun,
    plugin_ui_animation_free_fun_t fini_fun,
    plugin_ui_animation_enter_fun_t enter_fun,
    plugin_ui_animation_exit_fun_t exit_fun,
    plugin_ui_animation_update_fun_t update_fun,
    /*control*/
    size_t control_capacity,
    plugin_ui_animation_control_attach_fun_t control_attach,
    plugin_ui_animation_control_detach_fun_t control_detach,
    /*setup*/
    plugin_ui_animation_setup_fun_t setup);

void plugin_ui_animation_meta_free(plugin_ui_animation_meta_t meta);

plugin_ui_animation_meta_t
plugin_ui_animation_meta_find(plugin_ui_module_t module, const char * type_name);

#ifdef __cplusplus
}
#endif

#endif

