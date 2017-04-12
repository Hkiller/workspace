#ifndef UI_SPRITE_CHIPMUNK_MONITOR_I_H
#define UI_SPRITE_CHIPMUNK_MONITOR_I_H
#include "chipmunk/chipmunk_private.h"
#include "ui_sprite_chipmunk_obj_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ui_sprite_chipmunk_on_collision_fun_t)(
    void * ctx, UI_SPRITE_CHIPMUNK_COLLISION_DATA const * collision_data,
    ui_sprite_entity_t self_entity, ui_sprite_chipmunk_obj_body_t self_body,
    ui_sprite_entity_t other_entity, ui_sprite_chipmunk_obj_body_t other_body);

struct ui_sprite_chipmunk_monitor {
    ui_sprite_chipmunk_obj_t m_obj;
    TAILQ_ENTRY(ui_sprite_chipmunk_monitor) m_next_for_obj;
    ui_sprite_chipmunk_monitor_binding_list_t m_bindings;
    char * m_bodies;
    uint32_t m_collision_category;    
    uint32_t m_collision_mask;
    void * m_ctx;
    ui_sprite_chipmunk_on_collision_fun_t m_on_collision;
};

void ui_sprite_chipmunk_monitor_init(ui_sprite_chipmunk_module_t module, ui_sprite_chipmunk_monitor_t monitor);
void ui_sprite_chipmunk_monitor_fini(ui_sprite_chipmunk_module_t module, ui_sprite_chipmunk_monitor_t monitor);

int ui_sprite_chipmunk_monitor_copy(
    ui_sprite_chipmunk_module_t module,
    ui_sprite_chipmunk_monitor_t to_monitor, ui_sprite_chipmunk_monitor_t from_monitor);
    
int ui_sprite_chipmunk_monitor_enter(
    ui_sprite_chipmunk_obj_t obj, ui_sprite_chipmunk_monitor_t monitor);

void ui_sprite_chipmunk_monitor_exit(
    ui_sprite_chipmunk_monitor_t monitor);

int ui_sprite_chipmunk_monitor_load(
    ui_sprite_chipmunk_env_t env, ui_sprite_chipmunk_monitor_t monitor, cfg_t cfg);

#ifdef __cplusplus
}
#endif

#endif
