#ifndef UI_SPRITE_COMPONENT_META_H
#define UI_SPRITE_COMPONENT_META_H
#include "ui_sprite_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*ui_sprite_component_enter_fun_t)(ui_sprite_component_t component, void * ctx);
typedef void (*ui_sprite_component_exit_fun_t)(ui_sprite_component_t component, void * ctx);
typedef int (*ui_sprite_component_init_fun_t)(ui_sprite_component_t component, void * ctx);
typedef void (*ui_sprite_component_free_fun_t)(ui_sprite_component_t component, void * ctx);
typedef int (*ui_sprite_component_copy_fun_t)(ui_sprite_component_t to, ui_sprite_component_t from, void * ctx);
typedef void (*ui_sprite_component_update_fun_t)(ui_sprite_component_t component, void * ctx, float delta);

ui_sprite_component_meta_t
ui_sprite_component_meta_create(
    ui_sprite_repository_t repo, const char * name, uint16_t data_size);

void ui_sprite_component_meta_free(
    ui_sprite_component_meta_t component_meta);

ui_sprite_component_meta_t
ui_sprite_component_meta_find(
    ui_sprite_repository_t repo, const char * name);

void ui_sprite_component_meta_set_data_meta(
    ui_sprite_component_meta_t, LPDRMETA meta, uint16_t start, uint16_t size);

void ui_sprite_component_meta_set_enter_fun(
    ui_sprite_component_meta_t, ui_sprite_component_enter_fun_t fun, void * ctx);

void ui_sprite_component_meta_set_exit_fun(
    ui_sprite_component_meta_t, ui_sprite_component_exit_fun_t fun, void * ctx);

void ui_sprite_component_meta_set_init_fun(
    ui_sprite_component_meta_t, ui_sprite_component_init_fun_t fun, void * ctx);

void ui_sprite_component_meta_set_copy_fun(
    ui_sprite_component_meta_t, ui_sprite_component_copy_fun_t fun, void * ctx);

void ui_sprite_component_meta_set_free_fun(
    ui_sprite_component_meta_t, ui_sprite_component_free_fun_t fun, void * ctx);

void ui_sprite_component_meta_set_update_fun(
    ui_sprite_component_meta_t, ui_sprite_component_update_fun_t fun, void * ctx);

#ifdef __cplusplus
}
#endif

#endif
