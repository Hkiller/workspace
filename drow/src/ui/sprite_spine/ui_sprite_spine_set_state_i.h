#ifndef UI_SPRITE_SPINE_SET_STATE_I_H
#define UI_SPRITE_SPINE_SET_STATE_I_H
#include "render/utils/ui_percent_decorator.h"
#include "plugin/spine/plugin_spine_obj.h"
#include "ui/sprite_spine/ui_sprite_spine_set_state.h"
#include "ui_sprite_spine_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_spine_set_state {
    ui_sprite_spine_module_t m_module;
    char * m_cfg_part;
    char * m_cfg_state;
};

int ui_sprite_spine_set_state_regist(ui_sprite_spine_module_t module);
void ui_sprite_spine_set_state_unregist(ui_sprite_spine_module_t module);

#ifdef __cplusplus
}
#endif

#endif
