#ifndef UI_SPRITE_CHIPMUNK_WITH_BOUNDARY_I_H
#define UI_SPRITE_CHIPMUNK_WITH_BOUNDARY_I_H
#include "cpe/pal/pal_queue.h"
#include "plugin/chipmunk/plugin_chipmunk_data_fixture.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_with_boundary.h"
#include "ui_sprite_chipmunk_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

enum ui_sprite_chipmunk_with_boundary_build_mask {
    ui_sprite_chipmunk_with_boundary_build_left = 1u << 0,
    ui_sprite_chipmunk_with_boundary_build_right = 1u << 1,
    ui_sprite_chipmunk_with_boundary_build_top = 1u << 2,
    ui_sprite_chipmunk_with_boundary_build_bottom = 1u << 3,
};

struct ui_sprite_chipmunk_with_boundary_side {
    uint8_t m_need_build;
    float m_adj;
    float m_elasticity;
    float m_friction;
    uint32_t m_category;
    uint32_t m_mask;
    float m_radius;
};

struct ui_sprite_chipmunk_with_boundary {
    ui_sprite_chipmunk_module_t m_module;
    uint32_t m_category;
    uint32_t m_mask;
    enum ui_sprite_chipmunk_with_boundary_source_rect m_source_rect;
    struct ui_sprite_chipmunk_with_boundary_side m_top;
    struct ui_sprite_chipmunk_with_boundary_side m_bottom;
    struct ui_sprite_chipmunk_with_boundary_side m_left;
    struct ui_sprite_chipmunk_with_boundary_side m_right;
    ui_sprite_chipmunk_obj_body_t m_body;
};

int ui_sprite_chipmunk_with_boundary_regist(ui_sprite_chipmunk_module_t module);
void ui_sprite_chipmunk_with_boundary_unregist(ui_sprite_chipmunk_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
