#ifndef UI_SPRITE_CHIPMUNK_OBJ_BODY_GROUP_I_H
#define UI_SPRITE_CHIPMUNK_OBJ_BODY_GROUP_I_H
#include "ui_sprite_chipmunk_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_obj_body_group {
    ui_sprite_chipmunk_obj_body_group_binding_list_t m_bodies;
};

void ui_sprite_chipmunk_obj_body_group_init(ui_sprite_chipmunk_obj_body_group_t group);
void ui_sprite_chipmunk_obj_body_group_clear(ui_sprite_chipmunk_obj_body_group_t group);
void ui_sprite_chipmunk_obj_body_group_fini(ui_sprite_chipmunk_obj_body_group_t group);
uint8_t ui_sprite_chipmunk_obj_body_group_have_body(ui_sprite_chipmunk_obj_body_group_t group, ui_sprite_chipmunk_obj_body_t body);

struct ui_sprite_chipmunk_obj_body_group_binding {
    ui_sprite_chipmunk_obj_body_t m_body;
    ui_sprite_chipmunk_obj_body_group_t m_group;
    TAILQ_ENTRY(ui_sprite_chipmunk_obj_body_group_binding) m_next_for_body;
    TAILQ_ENTRY(ui_sprite_chipmunk_obj_body_group_binding) m_next_for_group;
};

ui_sprite_chipmunk_obj_body_group_binding_t
ui_sprite_chipmunk_obj_body_group_binding_create(
    ui_sprite_chipmunk_obj_body_group_t group,
    ui_sprite_chipmunk_obj_body_t body);
    
void ui_sprite_chipmunk_obj_body_group_binding_free(ui_sprite_chipmunk_obj_body_group_binding_t binding);    
void ui_sprite_chipmunk_obj_body_group_binding_real_free(ui_sprite_chipmunk_obj_body_group_binding_t binding);
    
#ifdef __cplusplus
}
#endif

#endif
