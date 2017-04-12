#ifndef UI_SPRITE_CHIPMUNK_WITH_TIME_SCALE_I_H
#define UI_SPRITE_CHIPMUNK_WITH_TIME_SCALE_I_H
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_with_time_scale.h"
#include "ui_sprite_chipmunk_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_with_time_scale {
    ui_sprite_chipmunk_module_t m_module;
    char * m_cfg_time_scale;
    float m_saved_time_scale;
};

int ui_sprite_chipmunk_with_time_scale_regist(ui_sprite_chipmunk_module_t module);
void ui_sprite_chipmunk_with_time_scale_unregist(ui_sprite_chipmunk_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
