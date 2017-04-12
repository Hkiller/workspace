#ifndef UI_SPRITE_CHIPMUNK_APPLY_VELOCITY_I_H
#define UI_SPRITE_CHIPMUNK_APPLY_VELOCITY_I_H
#include "cpe/pal/pal_queue.h"
#include "plugin/chipmunk/plugin_chipmunk_data_fixture.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_apply_velocity.h"
#include "ui_sprite_chipmunk_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_apply_velocity {
    ui_sprite_chipmunk_module_t m_module;
    char m_body_name[64];
    ui_sprite_chipmunk_unit_t m_unit;
    char * m_velocity_angle;
    char * m_velocity;
};

int ui_sprite_chipmunk_apply_velocity_regist(ui_sprite_chipmunk_module_t module);
void ui_sprite_chipmunk_apply_velocity_unregist(ui_sprite_chipmunk_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
