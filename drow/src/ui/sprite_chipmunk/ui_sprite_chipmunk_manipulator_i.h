#ifndef UI_SPRITE_CHIPMUNK_MANIPULATOR_I_H
#define UI_SPRITE_CHIPMUNK_MANIPULATOR_I_H
#include "cpe/utils/prand.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_manipulator.h"
#include "ui_sprite_chipmunk_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_manipulator {
    ui_sprite_chipmunk_module_t m_module;
};

int ui_sprite_chipmunk_manipulator_regist(ui_sprite_chipmunk_module_t module);
void ui_sprite_chipmunk_manipulator_unregist(ui_sprite_chipmunk_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
