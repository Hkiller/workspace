#ifndef UI_SPRITE_CHIPMUNK_WITH_GRAVITY_I_H
#define UI_SPRITE_CHIPMUNK_WITH_GRAVITY_I_H
#include "cpe/pal/pal_queue.h"
#include "plugin/chipmunk/plugin_chipmunk_data_fixture.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_with_gravity.h"
#include "ui_sprite_chipmunk_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_with_gravity {
    ui_sprite_chipmunk_module_t m_module;
    char m_body_name[64];
    char * m_gravity;
    char * m_gravity_angle;
    char * m_gravity_adj;
    UI_SPRITE_CHIPMUNK_GRAVITY m_saved_gravity;
};

int ui_sprite_chipmunk_with_gravity_regist(ui_sprite_chipmunk_module_t module);
void ui_sprite_chipmunk_with_gravity_unregist(ui_sprite_chipmunk_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
