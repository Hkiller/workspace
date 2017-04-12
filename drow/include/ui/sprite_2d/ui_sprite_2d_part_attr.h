#ifndef UI_SPRITE_2D_PART_ATTR_H
#define UI_SPRITE_2D_PART_ATTR_H
#include "ui_sprite_2d_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_2d_part_attr_it {
    ui_sprite_2d_part_attr_t (*next)(struct ui_sprite_2d_part_attr_it * it);
    char m_data[64];
};
    
ui_sprite_2d_part_attr_t ui_sprite_2d_part_attr_create(ui_sprite_2d_part_t part, const char * name);
void ui_sprite_2d_part_attr_free(ui_sprite_2d_part_attr_t part_attr);
    
ui_sprite_2d_part_attr_t ui_sprite_2d_part_attr_find(ui_sprite_2d_part_t part, const char * name);

const char * ui_sprite_2d_part_attr_name(ui_sprite_2d_part_attr_t part_attr);
const char * ui_sprite_2d_part_attr_value(ui_sprite_2d_part_attr_t part_attr);
int ui_sprite_2d_part_attr_set_value(ui_sprite_2d_part_attr_t part_attr, const char * value);
uint8_t ui_sprite_2d_part_attr_is_value_changed(ui_sprite_2d_part_attr_t part_attr);
    
void ui_sprite_2d_part_attrs(ui_sprite_2d_part_attr_it_t part_attr_it, ui_sprite_2d_part_t part);
    
#define ui_sprite_2d_part_attr_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
