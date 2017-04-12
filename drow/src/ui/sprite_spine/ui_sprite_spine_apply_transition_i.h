#ifndef UI_SPRITE_SPINE_APPLY_TRANSITION_I_H
#define UI_SPRITE_SPINE_APPLY_TRANSITION_I_H
#include "render/utils/ui_percent_decorator.h"
#include "plugin/spine/plugin_spine_obj.h"
#include "ui/sprite_spine/ui_sprite_spine_apply_transition.h"
#include "ui_sprite_spine_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_spine_apply_transition {
    ui_sprite_spine_module_t m_module;
    char * m_cfg_part;
    char * m_cfg_transition;
    char * m_part;
};

int ui_sprite_spine_apply_transition_regist(ui_sprite_spine_module_t module);
void ui_sprite_spine_apply_transition_unregist(ui_sprite_spine_module_t module);

#ifdef __cplusplus
}
#endif

#endif
