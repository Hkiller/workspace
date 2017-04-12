#ifndef UI_SPRITE_TOUCH_CLICK_I_H
#define UI_SPRITE_TOUCH_CLICK_I_H
#include "ui/sprite_touch/ui_sprite_touch_click.h"
#include "ui_sprite_touch_responser_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_touch_click {
    struct ui_sprite_touch_responser m_responser;
    char * m_on_click_down;
    char * m_on_click_up;
};

int ui_sprite_touch_click_regist(ui_sprite_touch_mgr_t module);
void ui_sprite_touch_click_unregist(ui_sprite_touch_mgr_t module);
ui_sprite_fsm_action_t ui_sprite_touch_click_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg);

#ifdef __cplusplus
}
#endif

#endif

