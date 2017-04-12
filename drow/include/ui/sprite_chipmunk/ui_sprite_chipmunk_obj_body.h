#ifndef UI_SPRITE_CHIPMUNK_OBJ_BODY_H
#define UI_SPRITE_CHIPMUNK_OBJ_BODY_H
#include "cpe/cfg/cfg_types.h"
#include "render/utils/ui_vector_2.h"
#include "ui_sprite_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_obj_body_it {
    ui_sprite_chipmunk_obj_body_t (*next)(struct ui_sprite_chipmunk_obj_body_it * it);
    char m_data[64];
};
    
ui_sprite_chipmunk_obj_body_t
ui_sprite_chipmunk_obj_body_create(ui_sprite_chipmunk_obj_t chipmunk_obj, uint32_t id, const char * name, uint8_t is_runtime);
ui_sprite_chipmunk_obj_body_t
ui_sprite_chipmunk_obj_body_create_from_data(ui_sprite_chipmunk_obj_t chipmunk_obj, plugin_chipmunk_data_body_t data_body, uint8_t is_runtime);

void ui_sprite_chipmunk_obj_body_free(ui_sprite_chipmunk_obj_body_t group);

const char * ui_sprite_chipmunk_obj_body_name(ui_sprite_chipmunk_obj_body_t body);    
ui_sprite_chipmunk_obj_t ui_sprite_chipmunk_obj_body_obj(ui_sprite_chipmunk_obj_body_t body);

void * ui_sprite_chipmunk_obj_body_cp_body(ui_sprite_chipmunk_obj_body_t body);
uint8_t ui_sprite_chipmunk_obj_body_is_in_space(ui_sprite_chipmunk_obj_body_t body);
uint8_t ui_sprite_chipmunk_obj_body_is_runtime(ui_sprite_chipmunk_obj_body_t body);
uint8_t ui_sprite_chipmunk_obj_body_is_main(ui_sprite_chipmunk_obj_body_t body);
int ui_sprite_chipmunk_obj_body_set_is_main(ui_sprite_chipmunk_obj_body_t body, uint8_t is_main);

uint8_t ui_sprite_chipmunk_obj_body_runing_mode(ui_sprite_chipmunk_obj_body_t body);
int ui_sprite_chipmunk_obj_body_set_runing_mode(ui_sprite_chipmunk_obj_body_t body, uint8_t runing_mode);

void ui_sprite_chipmunk_obj_body_collision_info(ui_sprite_chipmunk_obj_body_t body, uint32_t * category, uint32_t * mask, uint32_t * group_id);
void ui_sprite_chipmunk_obj_body_set_collision_category(ui_sprite_chipmunk_obj_body_t body, uint32_t category);
void ui_sprite_chipmunk_obj_body_set_collision_mask(ui_sprite_chipmunk_obj_body_t body, uint32_t mask);
    
chipmunk_obj_type_t ui_sprite_chipmunk_obj_body_type(ui_sprite_chipmunk_obj_body_t chipmunk_obj_body);
int ui_sprite_chipmunk_obj_body_set_type(ui_sprite_chipmunk_obj_body_t body, chipmunk_obj_type_t obj_type);

ui_vector_2 ui_sprite_chipmunk_obj_body_world_pos(ui_sprite_chipmunk_obj_body_t chipmunk_obj_body);
int ui_sprite_chipmunk_obj_body_set_local_pos(ui_sprite_chipmunk_obj_body_t chipmunk_obj_body, ui_vector_2_t pos);
int ui_sprite_chipmunk_obj_body_set_local_angle(ui_sprite_chipmunk_obj_body_t chipmunk_obj_body, float angle_rad);
    
void ui_sprite_chipmunk_obj_body_set_linear_velocity(ui_sprite_chipmunk_obj_body_t chipmunk_obj_body, float angle, float velocity);
void ui_sprite_chipmunk_obj_body_set_linear_velocity_pair(ui_sprite_chipmunk_obj_body_t chipmunk_obj_body, ui_vector_2 const * velocity_pair);
ui_vector_2 ui_sprite_chipmunk_obj_body_linear_velocity_pair(ui_sprite_chipmunk_obj_body_t chipmunk_obj_body);
float ui_sprite_chipmunk_obj_body_linear_velocity_angle(ui_sprite_chipmunk_obj_body_t chipmunk_obj_body);
float ui_sprite_chipmunk_obj_body_linear_velocity(ui_sprite_chipmunk_obj_body_t chipmunk_obj_body);
    
ui_sprite_chipmunk_obj_body_t ui_sprite_chipmunk_obj_body_find(ui_sprite_chipmunk_obj_t chipmunk_obj, const char * name);

UI_SPRITE_CHIPMUNK_GRAVITY const * ui_sprite_chipmunk_obj_body_gravity(ui_sprite_chipmunk_obj_body_t chipmunk_obj_body);
void ui_sprite_chipmunk_obj_body_set_gravity(ui_sprite_chipmunk_obj_body_t chipmunk_obj_body, UI_SPRITE_CHIPMUNK_GRAVITY const * gravity);

int ui_sprite_chipmunk_obj_body_add_to_space(ui_sprite_chipmunk_obj_body_t body);
void ui_sprite_chipmunk_obj_body_remove_from_space(ui_sprite_chipmunk_obj_body_t body);

void ui_sprite_chipmunk_obj_body_visit_shapes(
    ui_sprite_chipmunk_obj_body_t chipmunk_obj_body, ui_sprite_chipmunk_obj_shape_visit_fun_t fun, void * ctx);
    
#define ui_sprite_chipmunk_obj_body_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
