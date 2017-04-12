#ifndef UI_SPRITE_CHIPMUNK_OBJ_I_H
#define UI_SPRITE_CHIPMUNK_OBJ_I_H
#include "chipmunk/chipmunk_private.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_obj.h"
#include "ui_sprite_chipmunk_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_chipmunk_monitor * ui_sprite_chipmunk_monitor_t;
typedef TAILQ_HEAD(ui_sprite_chipmunk_monitor_list, ui_sprite_chipmunk_monitor) ui_sprite_chipmunk_monitor_list_t;
typedef struct ui_sprite_chipmunk_monitor_binding * ui_sprite_chipmunk_monitor_binding_t;
typedef TAILQ_HEAD(ui_sprite_chipmunk_monitor_binding_list, ui_sprite_chipmunk_monitor_binding) ui_sprite_chipmunk_monitor_binding_list_t;

typedef TAILQ_HEAD(ui_sprite_chipmunk_obj_body_list, ui_sprite_chipmunk_obj_body) ui_sprite_chipmunk_obj_body_list_t;
typedef TAILQ_HEAD(ui_sprite_chipmunk_obj_shape_group_list, ui_sprite_chipmunk_obj_shape_group) ui_sprite_chipmunk_obj_shape_group_list_t;
typedef TAILQ_HEAD(ui_sprite_chipmunk_obj_constraint_list, ui_sprite_chipmunk_obj_constraint) ui_sprite_chipmunk_obj_constraint_list_t;

struct ui_sprite_chipmunk_obj {
    ui_sprite_chipmunk_env_t m_env;
    UI_SPRITE_CHIPMUNK_OBJ_DATA m_data;
    ui_sprite_chipmunk_obj_body_t m_main_body;
    uint32_t m_body_count;
    ui_sprite_chipmunk_obj_body_list_t m_bodies;
    ui_sprite_chipmunk_obj_updator_list_t m_updators;
    ui_sprite_chipmunk_monitor_list_t m_monitors;
    ui_sprite_chipmunk_tri_scope_list_t m_scopes;
    ui_sprite_chipmunk_touch_responser_list_t m_responsers;
};

int ui_sprite_chipmunk_obj_regist(ui_sprite_chipmunk_module_t module);
void ui_sprite_chipmunk_obj_unregist(ui_sprite_chipmunk_module_t module);

int ui_sprite_component_obj_set_main_body(ui_sprite_chipmunk_obj_t chipmunk_obj, ui_sprite_chipmunk_obj_body_t body);
void ui_sprite_chipmunk_obj_update_move_policy(ui_sprite_chipmunk_obj_t chipmunk_obj);

int ui_sprite_chipmunk_obj_load(void * ctx, ui_sprite_component_t comp, cfg_t cfg);

#ifdef __cplusplus
}
#endif

#endif
