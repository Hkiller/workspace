#ifndef UI_SPRITE_SPINE_TRI_SET_TIMESCALE_H
#define UI_SPRITE_SPINE_TRI_SET_TIMESCALE_H
#include "ui_sprite_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SPINE_TRI_SET_TIMESCALE;

ui_sprite_spine_tri_set_timescale_t
ui_sprite_spine_tri_set_timescale_create(ui_sprite_tri_rule_t rule);

void ui_sprite_spine_tri_set_timescale_free(ui_sprite_spine_tri_set_timescale_t set_timescale);

plugin_spine_obj_t ui_sprite_spine_tri_set_timescale_obj(ui_sprite_spine_tri_set_timescale_t set_timescale);
void ui_sprite_spine_tri_set_timescale_set_obj(ui_sprite_spine_tri_set_timescale_t set_timescale, plugin_spine_obj_t obj);

const char * ui_sprite_spine_tri_set_timescale_part(ui_sprite_spine_tri_set_timescale_t set_timescale);
int ui_sprite_spine_tri_set_timescale_set_part(ui_sprite_spine_tri_set_timescale_t set_timescale, const char * part);
    
const char * ui_sprite_spine_tri_set_timescale_timescale(ui_sprite_spine_tri_set_timescale_t set_timescale);
int ui_sprite_spine_tri_set_timescale_set_timescale(ui_sprite_spine_tri_set_timescale_t set_timescale, const char * timescale);
    
#ifdef __cplusplus
}
#endif

#endif
