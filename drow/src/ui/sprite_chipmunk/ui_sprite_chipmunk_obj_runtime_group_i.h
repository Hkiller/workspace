#ifndef UI_SPRITE_CHIPMUNK_OBJ_RUNTIME_GROUP_I_H
#define UI_SPRITE_CHIPMUNK_OBJ_RUNTIME_GROUP_I_H
#include "ui_sprite_chipmunk_obj_body_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_obj_runtime_group {
    ui_sprite_chipmunk_obj_t m_body;
    uint32_t m_group_id;
    TAILQ_ENTRY(ui_sprite_chipmunk_obj_runtime_group) m_next_for_body;
    TAILQ_ENTRY(ui_sprite_chipmunk_obj_runtime_group) m_next_for_owner;    
};

ui_sprite_chipmunk_obj_runtime_group_t
ui_sprite_chipmunk_obj_runtime_group_create(
    ui_sprite_chipmunk_obj_runtime_group_list_t * owner,
    ui_sprite_chipmunk_obj_body_t body,
    uint32_t group_id);

void ui_sprite_chipmunk_obj_runtime_group_free(
    ui_sprite_chipmunk_module_t module,
    ui_sprite_chipmunk_obj_runtime_group_list_t * owner,
    ui_sprite_chipmunk_obj_runtime_group_t group);

void ui_sprite_chipmunk_obj_runtime_group_unbind(ui_sprite_chipmunk_obj_runtime_group_t group, ui_sprite_chipmunk_obj_body_t body);
    
#ifdef __cplusplus
}
#endif

#endif
