#ifndef UI_SPRITE_CHIPMUNK_OBJ_SHAPE_GROUP_I_H
#define UI_SPRITE_CHIPMUNK_OBJ_SHAPE_GROUP_I_H
#include "chipmunk/chipmunk_private.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_obj_shape_group.h"
#include "ui_sprite_chipmunk_obj_body_i.h"
#include "ui_sprite_chipmunk_obj_shape_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_chipmunk_obj_shape_buf * ui_sprite_chipmunk_obj_shape_buf_t;

struct ui_sprite_chipmunk_obj_shape_buf {
    ui_sprite_chipmunk_obj_shape m_shapes[64];
    struct ui_sprite_chipmunk_obj_shape_buf * m_next;
};

struct ui_sprite_chipmunk_obj_shape_group {
    ui_sprite_chipmunk_obj_body_t m_body;
    TAILQ_ENTRY(ui_sprite_chipmunk_obj_shape_group) m_next_for_body;
    uint32_t m_shape_count;
    ui_sprite_chipmunk_obj_shape m_inline_shapes[6];
    ui_sprite_chipmunk_obj_shape_buf_t m_bufs_begin;
    ui_sprite_chipmunk_obj_shape_buf_t m_bufs_last;
};

ui_sprite_chipmunk_obj_shape_group_t
ui_sprite_chipmunk_obj_shape_group_clone(ui_sprite_chipmunk_obj_body_t body, ui_sprite_chipmunk_obj_shape_group_t from_group);

int ui_sprite_chipmunk_obj_shape_group_init_shape(ui_sprite_chipmunk_obj_shape_group_t group, cpSpace * space, ui_sprite_2d_transform_t transform);
void ui_sprite_chipmunk_obj_shape_group_fini_shape(ui_sprite_chipmunk_obj_shape_group_t group);

#ifdef __cplusplus
}
#endif

#endif
