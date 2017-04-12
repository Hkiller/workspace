#ifndef UI_SPRITE_GROUP_H
#define UI_SPRITE_GROUP_H
#include "ui_sprite_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_group_t ui_sprite_group_create(ui_sprite_world_t world, const char * name);
void ui_sprite_group_free(ui_sprite_group_t group);

ui_sprite_group_t ui_sprite_group_find_by_id(ui_sprite_world_t world, uint32_t id);
ui_sprite_group_t ui_sprite_group_find_by_name(ui_sprite_world_t world, const char * name);

uint32_t ui_sprite_group_id(ui_sprite_group_t group);
const char * ui_sprite_group_name(ui_sprite_group_t group);
ui_sprite_world_t ui_sprite_group_world(ui_sprite_group_t gruop);

int ui_sprite_group_has_group(ui_sprite_group_t group, ui_sprite_group_t element);
int ui_sprite_group_has_group_r(ui_sprite_group_t group, ui_sprite_group_t element);
int ui_sprite_group_add_group(ui_sprite_group_t group, ui_sprite_group_t element);
void ui_sprite_group_remove_group(ui_sprite_group_t group, ui_sprite_group_t element);

int ui_sprite_group_has_entity(ui_sprite_group_t group, ui_sprite_entity_t element);
int ui_sprite_group_has_entity_r(ui_sprite_group_t group, ui_sprite_entity_t element);
int ui_sprite_group_add_entity(ui_sprite_group_t group, ui_sprite_entity_t element);
void ui_sprite_group_remove_entity(ui_sprite_group_t group, ui_sprite_entity_t element);

ui_sprite_entity_it_t ui_sprite_group_entities(mem_allocrator_t alloc, ui_sprite_group_t group);

ui_sprite_entity_t ui_sprite_group_first_entity(ui_sprite_group_t group);

typedef void (*ui_sprite_group_visit_fun_t)(ui_sprite_group_t g, ui_sprite_entity_t entity, void * visit_ctx);

void ui_sprite_group_visit(
    ui_sprite_group_t group, ui_sprite_group_visit_fun_t visit_fun, void * visit_ctx);

uint32_t ui_sprite_group_count(ui_sprite_group_t group);
uint8_t ui_sprite_group_is_empty(ui_sprite_group_t group);

void ui_sprite_group_send_event(
    ui_sprite_group_t group,
    LPDRMETA meta, void const * data, size_t size);

#ifdef __cplusplus
}
#endif

#endif
