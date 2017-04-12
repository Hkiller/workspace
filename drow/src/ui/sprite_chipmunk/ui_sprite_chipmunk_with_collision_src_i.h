#ifndef UI_SPRITE_CHIPMUNK_WITH_COLLISION_SRC_I_H
#define UI_SPRITE_CHIPMUNK_WITH_COLLISION_SRC_I_H
#include "ui_sprite_chipmunk_with_collision_shape_i.h"
#include "ui_sprite_chipmunk_load_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(ui_sprite_chipmunk_with_collision_src_list, ui_sprite_chipmunk_with_collision_src) ui_sprite_chipmunk_with_collision_src_list_t;
    
struct ui_sprite_chipmunk_with_collision_src {
    ui_sprite_chipmunk_with_collision_t m_with_collision;
    TAILQ_ENTRY(ui_sprite_chipmunk_with_collision_src) m_next;
    char m_res[128];
    char m_name[64];
    ui_sprite_chipmunk_with_collision_shape_list_t m_shapes;
    struct ui_sprite_chipmunk_body_attrs m_body_attrs;
    uint8_t m_is_main;
};

ui_sprite_chipmunk_with_collision_src_t
ui_sprite_chipmunk_with_collision_src_create(ui_sprite_chipmunk_with_collision_t with_collision);
void ui_sprite_chipmunk_with_collision_src_free(ui_sprite_chipmunk_with_collision_src_t src);
    
#ifdef __cplusplus
}
#endif

#endif
