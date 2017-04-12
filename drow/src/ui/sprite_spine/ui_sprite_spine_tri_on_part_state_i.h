#ifndef UI_SPRITE_CHIPMUNK_TRI_ON_PART_STATE_I_H
#define UI_SPRITE_CHIPMUNK_TRI_ON_PART_STATE_I_H
#include "ui/sprite_spine/ui_sprite_spine_tri_on_part_state.h"
#include "ui_sprite_spine_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_spine_tri_on_part_state {
    ui_sprite_spine_module_t m_module;
    plugin_spine_obj_t m_obj;
    char * m_part_name;
    char * m_part_state;
    uint8_t m_include_transition;
};

int ui_sprite_spine_tri_on_part_state_regist(ui_sprite_spine_module_t module);
void ui_sprite_spine_tri_on_part_state_unregist(ui_sprite_spine_module_t module);

#ifdef __cplusplus
}
#endif

#endif
