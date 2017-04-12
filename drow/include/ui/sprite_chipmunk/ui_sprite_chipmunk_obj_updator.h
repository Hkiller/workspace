#ifndef UI_SPRITE_CHIPMUNK_OBJ_UPDATOR_H
#define UI_SPRITE_CHIPMUNK_OBJ_UPDATOR_H
#include "ui_sprite_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_chipmunk_obj_updator_t
ui_sprite_chipmunk_obj_updator_create(
    ui_sprite_chipmunk_obj_t obj,
    ui_sprite_chipmunk_obj_updateor_update_fun_t update_fun,
    ui_sprite_chipmunk_obj_updateor_clean_fun_t clean_fun,
    size_t data_capacity);
    
void ui_sprite_chipmunk_obj_updator_free(ui_sprite_chipmunk_obj_updator_t updator);

void * ui_sprite_chipmunk_obj_updator_data(ui_sprite_chipmunk_obj_updator_t updator);

#ifdef __cplusplus
}
#endif

#endif
