#ifndef UI_SPRITE_RENDER_GROUP_I_H
#define UI_SPRITE_RENDER_GROUP_I_H
#include "render/utils/ui_transform.h"
#include "ui/sprite_2d/ui_sprite_2d_types.h"
#include "ui/sprite_render/ui_sprite_render_group.h"
#include "ui_sprite_render_sch_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_render_group {
    ui_sprite_render_sch_t m_sch;
    TAILQ_ENTRY(ui_sprite_render_group) m_next_for_sch;
    ui_sprite_render_anim_list_t m_anims;    
    const char * m_name;
    char * m_binding_part;
    ui_sprite_2d_part_t m_2d_part;
    uint8_t m_base_pos_of_entity;
	uint8_t m_base_pos_adj_policy;
    ui_transform m_trans;
    uint8_t m_accept_flip;
    uint8_t m_accept_scale;
	uint8_t m_accept_rotate;
    uint8_t m_adj_accept_scale;
    float m_adj_render_priority;
    ui_transform m_world_trans;
};

ui_sprite_render_group_t ui_sprite_render_group_clone(ui_sprite_render_sch_t render_sch, ui_sprite_render_group_t o);

int ui_sprite_render_group_enter(ui_sprite_render_group_t group, ui_sprite_2d_transform_t transform);
void ui_sprite_render_group_exit(ui_sprite_render_group_t group);

void ui_sprite_render_group_update_world_trans(ui_sprite_render_group_t group, ui_sprite_2d_transform_t transform);

#ifdef __cplusplus
}
#endif

#endif
