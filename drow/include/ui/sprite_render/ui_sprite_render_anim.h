#ifndef UI_SPRITE_RENDER_ANIM_H
#define UI_SPRITE_RENDER_ANIM_H
#include "render/runtime/ui_runtime_types.h"
#include "ui_sprite_render_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_render_anim_it {
    ui_sprite_render_anim_t (*next)(struct ui_sprite_render_anim_it * it);
    char m_data[64];
};
    
ui_sprite_render_anim_t
ui_sprite_render_anim_create(
    ui_sprite_render_layer_t layer, ui_runtime_render_obj_ref_t render_obj_ref,
    ui_sprite_render_group_t group, const char * name);

ui_sprite_render_anim_t
ui_sprite_render_anim_create_by_res(
    ui_sprite_render_layer_t layer, const char * res,
    ui_sprite_render_group_t group, const char * name);
    
void ui_sprite_render_anim_free(ui_sprite_render_anim_t anim);

uint32_t ui_sprite_render_anim_id(ui_sprite_render_anim_t anim);
ui_sprite_render_sch_t ui_sprite_render_anim_sch(ui_sprite_render_anim_t anim);
ui_sprite_render_group_t ui_sprite_render_anim_group(ui_sprite_render_anim_t anim);
ui_runtime_render_obj_ref_t ui_sprite_render_anim_obj(ui_sprite_render_anim_t anim);
ui_sprite_render_layer_t ui_sprite_render_anim_layer(ui_sprite_render_anim_t anim);
    
ui_sprite_render_anim_t ui_sprite_render_anim_find_by_id(ui_sprite_render_sch_t anim_sch, uint32_t anim_id);
ui_sprite_render_anim_t ui_sprite_render_anim_find_by_name(ui_sprite_render_sch_t anim_sch, const char * anim_name);
ui_sprite_render_anim_t ui_sprite_render_anim_find_by_render_obj(ui_sprite_render_sch_t anim_sch, ui_runtime_render_obj_t render_obj);
    
uint8_t ui_sprite_render_anim_is_runing(ui_sprite_render_anim_t anim);

ui_transform_t ui_sprite_render_anim_transform(ui_sprite_render_anim_t anim);
void ui_sprite_render_anim_set_transform(ui_sprite_render_anim_t anim, ui_transform_t transform);

int ui_sprite_render_anim_calc_obj_world_transform(ui_sprite_render_anim_t anim, ui_transform_t r);    
int ui_sprite_render_anim_calc_obj_local_transform(ui_sprite_render_anim_t anim, ui_transform_t r);
    
float ui_sprite_render_anim_priority(ui_sprite_render_anim_t anim);
void ui_sprite_render_anim_set_priority(ui_sprite_render_anim_t anim, float priority);

uint8_t ui_sprite_render_anim_sync_transform(ui_sprite_render_anim_t anim);
void ui_sprite_render_anim_set_sync_transform(ui_sprite_render_anim_t anim, uint8_t sync_transform);

uint8_t ui_sprite_render_anim_auto_remove(ui_sprite_render_anim_t anim);
void ui_sprite_render_anim_set_auto_remove(ui_sprite_render_anim_t anim, uint8_t auto_remove);

#define ui_sprite_render_anim_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
