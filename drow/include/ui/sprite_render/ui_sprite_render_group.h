#ifndef UI_SPRITE_RENDER_GROUP_H
#define UI_SPRITE_RENDER_GROUP_H
#include "render/utils/ui_vector_2.h"
#include "ui_sprite_render_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_render_group_it {
    ui_sprite_render_group_t (*next)(struct ui_sprite_render_group_it * it);
    char m_data[64];
};

ui_sprite_render_group_t ui_sprite_render_group_create(ui_sprite_render_sch_t render_sch, const char * name);
void ui_sprite_render_group_free(ui_sprite_render_group_t render_group);
ui_sprite_render_group_t ui_sprite_render_group_find_by_name(ui_sprite_render_sch_t render_sch, const char * name);

void ui_sprite_render_sch_groups(ui_sprite_render_group_it_t it, ui_sprite_render_sch_t sch);
    
uint8_t ui_sprite_render_group_accept_scale(ui_sprite_render_group_t group);
void ui_sprite_render_group_set_accept_scale(ui_sprite_render_group_t group, uint8_t accept_scale);

uint8_t ui_sprite_render_group_accept_rotate(ui_sprite_render_group_t group);
void ui_sprite_render_group_set_accept_rotate(ui_sprite_render_group_t group, uint8_t accept_rotate);

uint8_t ui_sprite_render_group_accept_flip(ui_sprite_render_group_t group);
void ui_sprite_render_group_set_accept_flip(ui_sprite_render_group_t group, uint8_t accept_flip);
    
uint8_t ui_sprite_render_group_adj_accept_scale(ui_sprite_render_group_t group);
void ui_sprite_render_group_set_adj_accept_scale(ui_sprite_render_group_t group, uint8_t pos_accept_scale);

uint8_t ui_sprite_render_group_base_pos(ui_sprite_render_group_t group);
void ui_sprite_render_group_set_base_pos(ui_sprite_render_group_t group, uint8_t base_pos);

ui_transform_t ui_sprite_render_group_local_trans(ui_sprite_render_group_t group);
void ui_sprite_render_group_set_local_trans(ui_sprite_render_group_t group, ui_transform_t local_trans);
    
uint8_t ui_sprite_render_group_base_pos_adj_policy(ui_sprite_render_group_t group);
void ui_sprite_render_group_set_base_pos_adj_policy(ui_sprite_render_group_t group, uint8_t base_pos_adj_policy);

float ui_sprite_render_group_adj_render_priority(ui_sprite_render_group_t group);
void ui_sprite_render_group_set_adj_render_priority(ui_sprite_render_group_t group, float adj_render_priority);

int ui_sprite_render_group_set_binding_part(ui_sprite_render_group_t group, const char * binding_part);
const char * ui_sprite_render_group_binding_part(ui_sprite_render_group_t group);

ui_transform_t ui_sprite_render_group_world_transform(ui_sprite_render_group_t group);
int ui_sprite_render_group_calc_local_transform(ui_sprite_render_group_t group, ui_transform_t r);
    
#define ui_sprite_render_group_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
