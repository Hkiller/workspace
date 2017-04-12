#ifndef UI_SPRITE_2D_UTILS_H
#define UI_SPRITE_2D_UTILS_H
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_rect.h"
#include "ui_sprite_2d_types.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t ui_sprite_2d_pt_in_rect(ui_vector_2 check_pt, ui_rect const * rect);
uint8_t ui_sprite_2d_pt_in_circle(ui_vector_2 check_pt, ui_vector_2 const * center, float radius);
uint8_t ui_sprite_2d_rect_in_rect(ui_rect const * check, ui_rect const * rect);

uint8_t ui_sprite_2d_pt_eq(ui_vector_2 p1, ui_vector_2 p2, float delta);
uint8_t ui_sprite_2d_rect_eq(ui_rect const * r1, ui_rect const * r2, float delta);

uint8_t ui_sprite_2d_rect_merge(ui_rect * target, ui_rect const * input);

int ui_sprite_2d_merge_contain_rect_group(ui_vector_2_t lt, ui_vector_2_t rb, ui_sprite_group_t group);
int ui_sprite_2d_merge_contain_rect_entity(ui_vector_2_t lt, ui_vector_2_t rb, ui_sprite_entity_t entity);

#ifdef __cplusplus
}
#endif

#endif


