#ifndef UI_SPRITE_RENDER_SCH_H
#define UI_SPRITE_RENDER_SCH_H
#include "ui_sprite_render_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_RENDER_SCH_NAME;

ui_sprite_render_sch_t ui_sprite_render_sch_create(ui_sprite_entity_t entity);
ui_sprite_render_sch_t ui_sprite_render_sch_find(ui_sprite_entity_t entity);
void ui_sprite_render_sch_free(ui_sprite_render_sch_t render_sch);

ui_sprite_render_env_t ui_sprite_render_sch_env(ui_sprite_render_sch_t render_sch);
ui_sprite_render_anim_t ui_sprite_render_anim_find(ui_sprite_render_env_t env, uint32_t anim_id);
    
float ui_sprite_render_sch_render_priority(ui_sprite_render_sch_t render_sch);
void ui_sprite_render_sch_set_render_priority(ui_sprite_render_sch_t render_sch, float render_priority);

ui_sprite_render_anim_t
ui_sprite_render_sch_start_anim(
    ui_sprite_render_sch_t render_sch, const char * group, const char * res, const char * name);

void ui_sprite_render_sch_set_default_layer(ui_sprite_render_sch_t render_sch, ui_sprite_render_layer_t layer);    
int ui_sprite_render_sch_set_default_layer_by_name(ui_sprite_render_sch_t render_sch, const char * layer_name);
ui_sprite_render_layer_t ui_sprite_render_sch_default_layer(ui_sprite_render_sch_t render_sch);
    
int ui_sprite_render_sch_add_renderation(ui_sprite_render_sch_t render_sch, const char * render_name, const char * res, uint8_t auto_start);

int ui_sprite_render_sch_setup(ui_sprite_render_sch_t render_sch, char * args);
    
#ifdef __cplusplus
}
#endif

#endif
