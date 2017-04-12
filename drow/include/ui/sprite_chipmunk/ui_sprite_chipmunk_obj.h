#ifndef UI_SPRITE_CHIPMUNK_OBJ_H
#define UI_SPRITE_CHIPMUNK_OBJ_H
#include "cpe/cfg/cfg_types.h"
#include "render/utils/ui_vector_2.h"
#include "ui_sprite_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CHIPMUNK_OBJ_NAME;

ui_sprite_chipmunk_obj_t ui_sprite_chipmunk_obj_create(ui_sprite_entity_t entity);
ui_sprite_chipmunk_obj_t ui_sprite_chipmunk_obj_find(ui_sprite_entity_t entity);
void ui_sprite_chipmunk_obj_free(ui_sprite_chipmunk_obj_t chipmunk_obj);

void ui_sprite_chipmunk_obj_set_linear_velocity(ui_sprite_chipmunk_obj_t chipmunk_obj, float angle, float velocity);
ui_sprite_chipmunk_obj_body_t ui_sprite_chipmunk_obj_main_body(ui_sprite_chipmunk_obj_t chipmunk_obj);
void ui_sprite_chipmunk_obj_bodies(ui_sprite_chipmunk_obj_body_it_t body_it, ui_sprite_chipmunk_obj_t chipmunk_obj);
void ui_sprite_chipmunk_obj_constraints(ui_sprite_chipmunk_obj_constraint_it_t constraint_it, ui_sprite_chipmunk_obj_t chipmunk_obj);

ui_sprite_chipmunk_env_t ui_sprite_chipmunk_obj_env(ui_sprite_chipmunk_obj_t chipmunk_obj);

uint8_t ui_sprite_chipmunk_obj_in_rect_bb(ui_sprite_chipmunk_obj_t chipmunk_obj, ui_rect_t rect);
    
uint8_t ui_sprite_chipmunk_obj_is_colllision_with(ui_sprite_chipmunk_obj_t chipmunk_obj, uint32_t mask);
uint8_t ui_sprite_chipmunk_obj_is_colllision_with_entity(ui_sprite_chipmunk_obj_t chipmunk_obj, ui_sprite_entity_t entity);

ui_sprite_chipmunk_obj_body_t
ui_sprite_chipmunk_obj_obj_load_body_tree_from_res(
    ui_sprite_chipmunk_obj_t chipmunk_obj,
    const char * res, const char * body_name, ui_vector_2 const * pos, uint8_t is_runtime);

ui_sprite_chipmunk_obj_body_t
ui_sprite_chipmunk_obj_load_body_tree(
    ui_sprite_chipmunk_obj_t chipmunk_obj,
    plugin_chipmunk_data_scene_t scene,
    const char * body_name, ui_vector_2 const * pos, uint8_t is_runtime);

ui_sprite_group_t
ui_sprite_chipmunk_obj_find_collision_entities(
    ui_sprite_chipmunk_obj_t chipmunk_obj, uint32_t mask);

#ifdef __cplusplus
}
#endif

#endif
