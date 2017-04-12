#ifndef UI_SPRITE_CHIPMUNK_OBJ_BODY_I_H
#define UI_SPRITE_CHIPMUNK_OBJ_BODY_I_H
#include "chipmunk/chipmunk_private.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_obj_body.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_load_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_obj_body_runtime_group {
    ui_sprite_chipmunk_obj_body_t m_body;
    uint32_t m_group;
};

struct ui_sprite_chipmunk_obj_body {
    ui_sprite_chipmunk_obj_t m_obj;
    TAILQ_ENTRY(ui_sprite_chipmunk_obj_body) m_next_for_obj;
    uint32_t m_id;
    char m_name[64];
    uint8_t m_is_runtime;
    uint8_t m_is_in_space;
    struct ui_sprite_chipmunk_body_attrs m_body_attrs;
    cpBody m_body;
    plugin_chipmunk_data_body_t m_load_from;
    ui_sprite_chipmunk_obj_runtime_group_list_t m_runtime_groups;
    ui_sprite_chipmunk_obj_constraint_list_t m_constraints_as_a;
    ui_sprite_chipmunk_obj_constraint_list_t m_constraints_as_b;
    ui_sprite_chipmunk_obj_shape_group_list_t m_shape_groups;
    ui_sprite_chipmunk_monitor_binding_list_t m_monitor_bindings;
    ui_sprite_chipmunk_obj_body_group_binding_list_t m_body_groups;
};

ui_sprite_chipmunk_obj_body_t
ui_sprite_chipmunk_obj_body_clone(ui_sprite_chipmunk_obj_t chipmunk_obj, ui_sprite_chipmunk_obj_body_t from_group);

uint8_t ui_sprite_chipmunk_obj_body_need_update(ui_sprite_chipmunk_obj_body_t body);

int ui_sprite_chipmunk_obj_body_add_to_space_i(
    ui_sprite_chipmunk_obj_body_t body, cpSpace * space, ui_sprite_2d_transform_t transform);

void ui_sprite_chipmunk_obj_body_update_collision(ui_sprite_chipmunk_obj_body_t body);

#ifdef __cplusplus
}
#endif

#endif
