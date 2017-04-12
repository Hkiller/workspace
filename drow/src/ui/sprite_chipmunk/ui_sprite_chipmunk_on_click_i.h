#ifndef UI_SPRITE_CHIPMUNK_ON_CLICK_I_H
#define UI_SPRITE_CHIPMUNK_ON_CLICK_I_H
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_on_click.h"
#include "ui_sprite_chipmunk_touch_responser_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_on_click {
    struct ui_sprite_chipmunk_touch_responser m_responser;
    char * m_on_click_down;
    char * m_on_click_up;
};

int ui_sprite_chipmunk_on_click_regist(ui_sprite_chipmunk_module_t module);
void ui_sprite_chipmunk_on_click_unregist(ui_sprite_chipmunk_module_t module);
ui_sprite_fsm_action_t ui_sprite_chipmunk_on_click_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg);

#ifdef __cplusplus
}
#endif

#endif

