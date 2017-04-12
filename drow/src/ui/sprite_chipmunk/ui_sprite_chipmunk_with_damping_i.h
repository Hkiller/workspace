#ifndef UI_SPRITE_CHIPMUNK_WITH_DAMPING_I_H
#define UI_SPRITE_CHIPMUNK_WITH_DAMPING_I_H
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_with_damping.h"
#include "ui_sprite_chipmunk_module_i.h"
#include "ui_sprite_chipmunk_obj_updator_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_with_damping {
    ui_sprite_chipmunk_module_t m_module;
    ui_sprite_chipmunk_obj_updator_t m_updator;
    const char * m_cfg_damping;
    const char * m_cfg_threshold;
    float m_threshold;
    float m_damping;
};

int ui_sprite_chipmunk_with_damping_regist(ui_sprite_chipmunk_module_t module);
void ui_sprite_chipmunk_with_damping_unregist(ui_sprite_chipmunk_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
