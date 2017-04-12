#ifndef UI_SPRITE_CHIPMUNK_CHIPMUNK_BODY_I_H
#define UI_SPRITE_CHIPMUNK_CHIPMUNK_BODY_I_H
#include "render/utils/ui_vector_2.h"
#include "ui_sprite_spine_chipmunk_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ui_sprite_spine_chipmunk_body_state {
    ui_sprite_spine_chipmunk_body_state_active,
    ui_sprite_spine_chipmunk_body_state_colliede,
} ui_sprite_spine_chipmunk_body_state_t;
    
struct ui_sprite_spine_chipmunk_body {
    ui_sprite_spine_chipmunk_with_collision_t m_with_collision;
    TAILQ_ENTRY(ui_sprite_spine_chipmunk_body) m_next;
    ui_sprite_spine_chipmunk_body_state_t m_state;
    char m_name[64];
    struct spSlot * m_slot;
    struct spAttachment * m_attachment;
    ui_vector_2 m_scale;
    ui_sprite_chipmunk_obj_body_t m_chipmunk_body;
};

/*return need remove*/
uint8_t ui_sprite_spine_chipmunk_body_on_collided(ui_sprite_spine_chipmunk_body_t body, ui_vector_2_t pt);

ui_sprite_spine_chipmunk_body_t
ui_sprite_spine_chipmunk_body_create(
    ui_sprite_spine_chipmunk_with_collision_t with_collision, const char * name, struct spSlot * slot, struct spAttachment * attachment);

void ui_sprite_spine_chipmunk_body_free(ui_sprite_spine_chipmunk_body_t body);

void ui_sprite_spine_chipmunk_body_update(ui_sprite_chipmunk_obj_t chipmunk_obj, ui_sprite_spine_chipmunk_body_t body, ui_transform_t local_trans);

#ifdef __cplusplus
}
#endif

#endif
