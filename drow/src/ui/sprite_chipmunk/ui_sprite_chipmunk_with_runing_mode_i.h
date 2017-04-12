#ifndef UI_SPRITE_CHIPMUNK_WITH_RUNING_MODE_I_H
#define UI_SPRITE_CHIPMUNK_WITH_RUNING_MODE_I_H
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_with_runing_mode.h"
#include "ui_sprite_chipmunk_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_with_runing_mode {
    ui_sprite_chipmunk_module_t m_module;
    char * m_cfg_runing_mode;
    char * m_cfg_body_name;
    char * m_body_name;
    ui_sprite_chipmunk_runing_mode_t m_saved_runing_mode;
};

int ui_sprite_chipmunk_with_runing_mode_regist(ui_sprite_chipmunk_module_t module);
void ui_sprite_chipmunk_with_runing_mode_unregist(ui_sprite_chipmunk_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
