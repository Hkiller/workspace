#ifndef UI_SPRITE_CHIPMUNK_TRI_HAVE_ENTITY_H
#define UI_SPRITE_CHIPMUNK_TRI_HAVE_ENTITY_H
#include "ui_sprite_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CHIPMUNK_TRI_HAVE_ENTITY;

ui_sprite_chipmunk_tri_have_entity_t
ui_sprite_chipmunk_tri_have_entity_create(ui_sprite_tri_rule_t rule);
    
void ui_sprite_chipmunk_tri_have_entity_free(ui_sprite_chipmunk_tri_have_entity_t have_entity);

ui_sprite_chipmunk_obj_t ui_sprite_chipmunk_tri_have_entity_obj(ui_sprite_chipmunk_tri_have_entity_t have_entity);
    
ui_sprite_chipmunk_tri_scope_t ui_sprite_chipmunk_tri_have_entity_scope(ui_sprite_chipmunk_tri_have_entity_t have_entity);

const char * ui_sprite_chipmunk_tri_have_entity_name_pattern(ui_sprite_chipmunk_tri_have_entity_t have_entity);
int ui_sprite_chipmunk_tri_have_entity_set_name_pattern(ui_sprite_chipmunk_tri_have_entity_t have_entity, const char * pattern);

const char * ui_sprite_chipmunk_tri_have_entity_require_count(ui_sprite_chipmunk_tri_have_entity_t have_entity);
int ui_sprite_chipmunk_tri_have_entity_set_require_count(ui_sprite_chipmunk_tri_have_entity_t have_entity, const char * require_count);
    
#ifdef __cplusplus
}
#endif

#endif
