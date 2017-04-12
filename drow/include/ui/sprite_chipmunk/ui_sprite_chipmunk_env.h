#ifndef UI_SPRITE_CHIPMUNK_ENV_H
#define UI_SPRITE_CHIPMUNK_ENV_H
#include "ui_sprite_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CHIPMUNK_ENV_NAME;

ui_sprite_chipmunk_env_t ui_sprite_chipmunk_env_create(ui_sprite_chipmunk_module_t module, ui_sprite_world_t world);
void ui_sprite_chipmunk_env_free(ui_sprite_world_t world);

ui_sprite_chipmunk_env_t ui_sprite_chipmunk_env_find(ui_sprite_world_t world);

plugin_chipmunk_env_t ui_sprite_chipmunk_env_env(ui_sprite_chipmunk_env_t env);
ui_sprite_chipmunk_module_t ui_sprite_chipmunk_env_module(ui_sprite_chipmunk_env_t env);

uint32_t ui_sprite_chipmunk_env_collision_type(ui_sprite_chipmunk_env_t env);

int ui_sprite_chipmunk_env_set_update_priority(ui_sprite_chipmunk_env_t env, int8_t priority);

int ui_sprite_chipmunk_env_set_process_touch(ui_sprite_chipmunk_env_t env, uint8_t is_process_touch);
uint8_t sprite_chipmunk_env_is_process_touch(ui_sprite_chipmunk_env_t env);

int ui_sprite_chipmunk_env_query_bodies_by_shape(
    ui_sprite_chipmunk_env_t env, ui_sprite_entity_t from_entity,
    ui_sprite_chipmunk_obj_body_visit_fun_t fun, void * ctx,
    CHIPMUNK_SHAPE const * shape, uint32_t category, uint32_t mask, uint32_t gruop);
        
int ui_sprite_chipmunk_env_query_bodies_by_point(
    ui_sprite_chipmunk_env_t env, 
    ui_sprite_chipmunk_obj_body_visit_fun_t fun, void * ctx,
    CHIPMUNK_PAIR const * pos, float radius, uint32_t category, uint32_t mask, uint32_t gruop);

ui_sprite_group_t
ui_sprite_chipmunk_env_query_entities_by_shape(
    ui_sprite_chipmunk_env_t env,
    ui_sprite_entity_t from_entity, CHIPMUNK_SHAPE const * from_shape,
    uint32_t category, uint32_t mask, uint32_t gruop);

#ifdef __cplusplus
}
#endif

#endif
