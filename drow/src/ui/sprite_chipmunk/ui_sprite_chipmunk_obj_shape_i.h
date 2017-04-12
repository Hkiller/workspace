#ifndef UI_SPRITE_CHIPMUNK_OBJ_SHAPE_I_H
#define UI_SPRITE_CHIPMUNK_OBJ_SHAPE_I_H
#include "chipmunk/chipmunk_private.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_obj_shape.h"
#include "ui_sprite_chipmunk_obj_body_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_obj_shape_node_buf {
    uint32_t m_capacity;
    uint32_t m_count;
    CHIPMUNK_PAIR m_poss[1];
};

typedef struct ui_sprite_chipmunk_obj_shape {
    union {
        struct cpCircleShape m_circle;
        struct cpSegmentShape m_segment;
        struct cpPolyShape m_poly;
    };
    ui_sprite_chipmunk_obj_shape_group_t m_group;
    uint8_t m_is_managed;
    uint8_t m_is_generated;
    CHIPMUNK_FIXTURE * m_fixture_data;
    plugin_chipmunk_data_polygon_node_t m_polygon_node;
    uint16_t m_polygon_node_count;
    struct ui_sprite_chipmunk_obj_shape_node_buf * m_inline_nodes;
} ui_sprite_chipmunk_obj_shape;

ui_sprite_chipmunk_obj_shape_t
ui_sprite_chipmunk_obj_shape_create_i(
    ui_sprite_chipmunk_obj_shape_group_t group,
    CHIPMUNK_FIXTURE * fixture_data, plugin_chipmunk_data_polygon_node_t polygon_node, uint16_t polygon_node_count, uint8_t is_managed);

void ui_sprite_chipmunk_obj_shape_free_i(ui_sprite_chipmunk_obj_shape_t shape);

int ui_sprite_chipmunk_obj_shape_init(ui_sprite_chipmunk_obj_shape_t obj_shape, cpSpace * space, ui_sprite_2d_transform_t transform);
void ui_sprite_chipmunk_obj_shape_fini(ui_sprite_chipmunk_obj_shape_t obj_shape);
    
#ifdef __cplusplus
}
#endif

#endif
