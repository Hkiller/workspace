#ifndef UI_SPRITE_WORLD_RES_H
#define UI_SPRITE_WORLD_RES_H
#include "ui_sprite_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ui_sprite_world_res_free_fun_t)(ui_sprite_world_res_t world_res, void * ctx);

ui_sprite_world_res_t ui_sprite_world_res_create(ui_sprite_world_t world, const char * name, size_t size);
void ui_sprite_world_res_free(ui_sprite_world_res_t world);

ui_sprite_world_res_t ui_sprite_world_res_find(ui_sprite_world_t world, const char * name);

ui_sprite_world_t ui_sprite_world_res_world(ui_sprite_world_res_t world_res);
void * ui_sprite_world_res_data(ui_sprite_world_res_t world_res);
size_t ui_sprite_world_res_data_size(ui_sprite_world_res_t world_res);

ui_sprite_world_res_t ui_sprite_world_res_from_data(void * data);

void ui_sprite_world_res_set_free_fun(
    ui_sprite_world_res_t, ui_sprite_world_res_free_fun_t fun, void * ctx);

#ifdef __cplusplus
}
#endif

#endif
