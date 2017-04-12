#ifndef UI_SPRITE_2D_PART_H
#define UI_SPRITE_2D_PART_H
#include "ui_sprite_2d_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_2d_part_it {
    ui_sprite_2d_part_t (*next)(struct ui_sprite_2d_part_it * it);
    char m_data[64];
};
    
ui_sprite_2d_part_t ui_sprite_2d_part_create(ui_sprite_2d_transform_t transform, const char * name);
void ui_sprite_2d_part_free(ui_sprite_2d_part_t part);
    
ui_sprite_2d_part_t ui_sprite_2d_part_find(ui_sprite_2d_transform_t transform, const char * name);
ui_sprite_2d_part_t ui_sprite_2d_part_check_create(ui_sprite_2d_transform_t transform, const char * name);
    
const char * ui_sprite_2d_part_name(ui_sprite_2d_part_t part);
ui_sprite_2d_transform_t ui_sprite_2d_part_transform(ui_sprite_2d_part_t part);

ui_transform_t ui_sprite_2d_part_trans(ui_sprite_2d_part_t part);
void ui_sprite_2d_part_set_trans(ui_sprite_2d_part_t part, ui_transform_t trans);
int ui_sprite_2d_part_set_world_trans(ui_sprite_2d_part_t part, ui_transform_t trans);

const char * ui_sprite_2d_part_value(ui_sprite_2d_part_t part, const char * attr_name, const char * dft);
int ui_sprite_2d_part_set_value(ui_sprite_2d_part_t part, const char * attr_name, const char * value);
int ui_sprite_2d_part_bulk_set_values_mutable(ui_sprite_2d_part_t part, char * defs, char sep /*,*/, char pair /*=*/);
    
int ui_sprite_2d_part_calc_world_trans(ui_sprite_2d_part_t part, ui_transform_t world_trans);

void ui_sprite_2d_transform_parts(ui_sprite_2d_part_it_t part_it, ui_sprite_2d_transform_t transform);

uint8_t ui_sprite_2d_part_attr_updated(ui_sprite_2d_part_t part);
uint8_t ui_sprite_2d_part_trans_updated(ui_sprite_2d_part_t part);
void ui_sprite_2d_part_dispatch_event(ui_sprite_2d_part_t part);
    
#define ui_sprite_2d_part_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
