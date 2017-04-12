#ifndef UI_SPRITE_CHIPMUNK_OBJ_SHAPE_GROUP_H
#define UI_SPRITE_CHIPMUNK_OBJ_SHAPE_GROUP_H
#include "cpe/cfg/cfg_types.h"
#include "ui_sprite_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_chipmunk_obj_shape_group_t
ui_sprite_chipmunk_obj_shape_group_create(ui_sprite_chipmunk_obj_body_t body);
ui_sprite_chipmunk_obj_shape_group_t    
ui_sprite_chipmunk_obj_shape_group_create_from_data(
    ui_sprite_chipmunk_obj_body_t body, plugin_chipmunk_data_fixture_t fixture);
void ui_sprite_chipmunk_obj_shape_group_free(ui_sprite_chipmunk_obj_shape_group_t group);

void ui_sprite_chipmunk_obj_shape_group_visit_shapes(ui_sprite_chipmunk_obj_shape_group_t group, ui_sprite_chipmunk_obj_shape_visit_fun_t fun, void * ctx);

ui_sprite_chipmunk_obj_body_t ui_sprite_chipmunk_obj_shape_group_body(ui_sprite_chipmunk_obj_shape_group_t shape_group);

#ifdef __cplusplus
}
#endif

#endif
