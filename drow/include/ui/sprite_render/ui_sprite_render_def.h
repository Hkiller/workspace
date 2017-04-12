#ifndef UI_SPRITE_RENDER_DEF_H
#define UI_SPRITE_RENDER_DEF_H
#include "ui_sprite_render_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_render_def_it {
    ui_sprite_render_def_t (*next)(struct ui_sprite_render_def_it * it);
    char m_data[64];
};

ui_sprite_render_def_t
ui_sprite_render_def_create(
    ui_sprite_render_sch_t render_sch, const char * anim_name, const char * res, uint8_t auto_start);

void ui_sprite_render_def_free(ui_sprite_render_def_t def);

ui_sprite_render_def_t
ui_sprite_render_def_find(ui_sprite_render_sch_t render_sch, const char * name);

uint8_t ui_sprite_render_def_auto_start(ui_sprite_render_def_t render_def);
const char * ui_sprite_render_def_anim_name(ui_sprite_render_def_t render_def);
const char * ui_sprite_render_def_anim_res(ui_sprite_render_def_t render_def);
ui_data_src_t ui_sprite_render_def_anim_src(ui_sprite_render_def_t render_def);
    
void ui_sprite_render_sch_defs(ui_sprite_render_def_it_t it, ui_sprite_render_sch_t render_sch);

#define ui_sprite_render_def_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
