#ifndef UI_SPRITE_SPINE_TRI_APPLY_TRANSITION_I_H
#define UI_SPRITE_SPINE_TRI_APPLY_TRANSITION_I_H
#include "ui/sprite_spine/ui_sprite_spine_tri_apply_transition.h"
#include "ui_sprite_spine_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_spine_tri_apply_transition {
    ui_sprite_spine_module_t m_module;
    plugin_spine_obj_t m_obj;
    char * m_part;
    char * m_transition;
};

int ui_sprite_spine_tri_apply_transition_regist(ui_sprite_spine_module_t module);
void ui_sprite_spine_tri_apply_transition_unregist(ui_sprite_spine_module_t module);

#ifdef __cplusplus
}
#endif

#endif
