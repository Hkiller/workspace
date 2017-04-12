#ifndef UI_SPRITE_CHIPMUNK_WITH_COLLISION_BODY_I_H
#define UI_SPRITE_CHIPMUNK_WITH_COLLISION_BODY_I_H
#include "cpe/pal/pal_queue.h"
#include "plugin/chipmunk/plugin_chipmunk_data_fixture.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_chipmunk_with_collision_body * ui_sprite_chipmunk_with_collision_body_t;
typedef TAILQ_HEAD(ui_sprite_chipmunk_with_collision_body_list, ui_sprite_chipmunk_with_collision_body) ui_sprite_chipmunk_with_collision_body_list_t;

struct ui_sprite_chipmunk_with_collision_body {
    TAILQ_ENTRY(ui_sprite_chipmunk_with_collision_body) m_next;
    ui_sprite_chipmunk_obj_body_t m_loaded_body;
};

ui_sprite_chipmunk_with_collision_body_t
ui_sprite_chipmunk_with_collision_body_create(
    ui_sprite_entity_t entity, ui_sprite_chipmunk_obj_t chipmunk_obj, ui_sprite_chipmunk_with_collision_t with_collision,
    ui_sprite_2d_transform_t transform, cpSpace * space,
    plugin_chipmunk_data_body_t data_body, ui_sprite_chipmunk_with_collision_src_t src);

void ui_sprite_chipmunk_with_collision_body_free(
    ui_sprite_chipmunk_with_collision_t with_collision, ui_sprite_chipmunk_with_collision_body_t body);
    
#ifdef __cplusplus
}
#endif

#endif
