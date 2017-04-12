#ifndef UI_SPRITE_2D_PART_BINDING_H
#define UI_SPRITE_2D_PART_BINDING_H
#include "ui_sprite_2d_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ui_sprite_2d_part_on_updated_fun_t)(ui_sprite_2d_part_t part, void * ctx);
    
ui_sprite_2d_part_binding_t
ui_sprite_2d_part_binding_create(
    ui_sprite_2d_part_t part,
    void * ctx,
    ui_sprite_2d_part_on_updated_fun_t on_updated);
    
void ui_sprite_2d_part_binding_free(ui_sprite_2d_part_binding_t part_binding);

void ui_sprite_2d_part_remove_bindings(ui_sprite_2d_part_t part, void * ctx);
void ui_sprite_2d_transform_remove_bindings(ui_sprite_2d_transform_t transform, void * ctx);
    
#ifdef __cplusplus
}
#endif

#endif
