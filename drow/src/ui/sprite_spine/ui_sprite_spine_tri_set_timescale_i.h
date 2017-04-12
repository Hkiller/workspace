#ifndef UI_SPRITE_SPINE_TRI_SET_TIMESCALE_I_H
#define UI_SPRITE_SPINE_TRI_SET_TIMESCALE_I_H
#include "ui/sprite_spine/ui_sprite_spine_tri_set_timescale.h"
#include "ui_sprite_spine_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_spine_tri_set_timescale {
    ui_sprite_spine_module_t m_module;
    plugin_spine_obj_t m_obj;
    char * m_part;
    char * m_timescale;
};

int ui_sprite_spine_tri_set_timescale_regist(ui_sprite_spine_module_t module);
void ui_sprite_spine_tri_set_timescale_unregist(ui_sprite_spine_module_t module);

#ifdef __cplusplus
}
#endif

#endif
