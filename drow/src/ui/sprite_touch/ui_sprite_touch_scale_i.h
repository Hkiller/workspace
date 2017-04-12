#ifndef UI_SPRITE_TOUCH_SCALE_I_H
#define UI_SPRITE_TOUCH_SCALE_I_H
#include "ui/sprite_touch/ui_sprite_touch_scale.h"
#include "ui_sprite_touch_responser_i.h"
#include "protocol/ui/sprite_touch/ui_sprite_touch_evt.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_touch_scale {
    struct ui_sprite_touch_responser m_responser;
    ui_sprite_touch_mgr_t m_mgr;
    char * m_on_begin;
    char * m_on_scale;
    char * m_on_end;
    char * m_on_cancel;
    UI_SPRITE_TOUCH_SCALE_STATE m_state;
};

int ui_sprite_touch_scale_regist(ui_sprite_touch_mgr_t module);
void ui_sprite_touch_scale_unregist(ui_sprite_touch_mgr_t module);
ui_sprite_fsm_action_t ui_sprite_touch_scale_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg);

#ifdef __cplusplus
}
#endif

#endif

