#ifndef UI_SPRITE_FSM_ACTION_META_H
#define UI_SPRITE_FSM_ACTION_META_H
#include "ui_sprite_fsm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*ui_sprite_fsm_action_enter_fun_t)(ui_sprite_fsm_action_t fsm_action, void * ctx);
typedef void (*ui_sprite_fsm_action_exit_fun_t)(ui_sprite_fsm_action_t fsm_action, void * ctx);
typedef int (*ui_sprite_fsm_action_init_fun_t)(ui_sprite_fsm_action_t fsm_action, void * ctx);
typedef void (*ui_sprite_fsm_action_free_fun_t)(ui_sprite_fsm_action_t fsm_action, void * ctx);
typedef int (*ui_sprite_fsm_action_copy_fun_t)(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx);
typedef void (*ui_sprite_fsm_action_update_fun_t)(ui_sprite_fsm_action_t to, void * ctx, float delta_s);

ui_sprite_fsm_action_meta_t
ui_sprite_fsm_action_meta_create(ui_sprite_fsm_module_t module, const char * name, uint16_t data_size);

const char * ui_sprite_fsm_action_meta_name(ui_sprite_fsm_action_meta_t meta);

void ui_sprite_fsm_action_meta_free(ui_sprite_fsm_action_meta_t fsm_action_meta);

ui_sprite_fsm_action_meta_t ui_sprite_fsm_action_meta_find(ui_sprite_fsm_module_t module, const char * name);

void ui_sprite_fsm_action_meta_set_data_meta(
    ui_sprite_fsm_action_meta_t, LPDRMETA meta, uint16_t start, uint16_t size);

void ui_sprite_fsm_action_meta_set_init_fun(
    ui_sprite_fsm_action_meta_t, ui_sprite_fsm_action_init_fun_t fun, void * ctx);

void ui_sprite_fsm_action_meta_set_copy_fun(
    ui_sprite_fsm_action_meta_t, ui_sprite_fsm_action_copy_fun_t fun, void * ctx);

void ui_sprite_fsm_action_meta_set_free_fun(
    ui_sprite_fsm_action_meta_t, ui_sprite_fsm_action_free_fun_t fun, void * ctx);

void ui_sprite_fsm_action_meta_set_enter_fun(
    ui_sprite_fsm_action_meta_t, ui_sprite_fsm_action_enter_fun_t fun, void * ctx);

void ui_sprite_fsm_action_meta_set_exit_fun(
    ui_sprite_fsm_action_meta_t, ui_sprite_fsm_action_exit_fun_t fun, void * ctx);

void ui_sprite_fsm_action_meta_set_update_fun(
    ui_sprite_fsm_action_meta_t, ui_sprite_fsm_action_update_fun_t fun, void * ctx);

#ifdef __cplusplus
}
#endif

#endif
