#ifndef UI_SPRITE_TRI_ACTION_META_H
#define UI_SPRITE_TRI_ACTION_META_H
#include "ui_sprite_tri_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*ui_sprite_tri_action_init_fun_t)(void * ctx, ui_sprite_tri_action_t action);
typedef int (*ui_sprite_tri_action_copy_fun_t)(void * ctx, ui_sprite_tri_action_t action, ui_sprite_tri_action_t source);
typedef void (*ui_sprite_tri_action_fini_fun_t)(void * ctx, ui_sprite_tri_action_t action);
typedef void (*ui_sprite_tri_action_exec_fun_t)(void * ctx, ui_sprite_tri_action_t action);    

ui_sprite_tri_action_meta_t
ui_sprite_tri_action_meta_create(
    ui_sprite_tri_module_t module, const char * name, size_t data_capacity,
    void * ctx,
    ui_sprite_tri_action_init_fun_t init,
    ui_sprite_tri_action_fini_fun_t fini,
    ui_sprite_tri_action_copy_fun_t copy,
    ui_sprite_tri_action_exec_fun_t check);

void ui_sprite_tri_action_meta_free(ui_sprite_tri_action_meta_t meta);

ui_sprite_tri_action_meta_t
ui_sprite_tri_action_meta_find(ui_sprite_tri_module_t module, const char * name);
    
#ifdef __cplusplus
}
#endif

#endif
