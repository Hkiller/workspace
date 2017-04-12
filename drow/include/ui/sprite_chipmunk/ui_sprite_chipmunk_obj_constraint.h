#ifndef UI_SPRITE_CHIPMUNK_OBJ_CONSTRAINT_H
#define UI_SPRITE_CHIPMUNK_OBJ_CONSTRAINT_H
#include "cpe/cfg/cfg_types.h"
#include "ui_sprite_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_obj_constraint_it {
    ui_sprite_chipmunk_obj_constraint_t (*next)(struct ui_sprite_chipmunk_obj_constraint_it * it);
    char m_data[64];
};
    
ui_sprite_chipmunk_obj_constraint_t
ui_sprite_chipmunk_obj_constraint_create(
    ui_sprite_chipmunk_obj_body_t body_a, ui_sprite_chipmunk_obj_body_t body_b,
    const char * constraint_name, uint8_t constraint_type, CHIPMUNK_CONSTRAINT_DATA const * constraint_data, uint8_t is_runtime);

void ui_sprite_chipmunk_obj_constraint_free(ui_sprite_chipmunk_obj_constraint_t constraint);

const char * ui_sprite_chipmunk_obj_constraint_name(ui_sprite_chipmunk_obj_constraint_t constraint);

uint8_t ui_sprite_chipmunk_obj_constraint_type(ui_sprite_chipmunk_obj_constraint_t constraint);
CHIPMUNK_CONSTRAINT_DATA * ui_sprite_chipmunk_obj_constraint_data(ui_sprite_chipmunk_obj_constraint_t constraint);    
    
ui_sprite_chipmunk_obj_constraint_t ui_sprite_chipmunk_obj_constraint_find(ui_sprite_chipmunk_obj_body_t body, const char * name);
ui_sprite_chipmunk_obj_constraint_t ui_sprite_chipmunk_obj_constraint_find_as_a(ui_sprite_chipmunk_obj_body_t body, const char * name);
ui_sprite_chipmunk_obj_constraint_t ui_sprite_chipmunk_obj_constraint_find_as_b(ui_sprite_chipmunk_obj_body_t body, const char * name);
    
#define ui_sprite_chipmunk_obj_constraint_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
