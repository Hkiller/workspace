#ifndef UI_SPRITE_CHIPMUNK_TOUCH_MOVE_I_H
#define UI_SPRITE_CHIPMUNK_TOUCH_MOVE_I_H
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_touch_move.h"
#include "ui_sprite_chipmunk_touch_responser_i.h"
#include "protocol/ui/sprite_chipmunk/ui_sprite_chipmunk_evt.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_touch_move {
    struct ui_sprite_chipmunk_touch_responser m_responser;
    ui_sprite_chipmunk_module_t m_mgr;
    char * m_on_begin;
    char * m_on_move;
    char * m_on_end;
    char * m_on_cancel;
    uint8_t m_finger_count;
    uint8_t m_is_grab;
    uint8_t m_is_capture;
    uint16_t m_threshold;
    float m_stick_duration;
    UI_SPRITE_CHIPMUNK_MOVE_STATE m_state;
};

int ui_sprite_chipmunk_touch_move_regist(ui_sprite_chipmunk_module_t module);
void ui_sprite_chipmunk_touch_move_unregist(ui_sprite_chipmunk_module_t module);
    
ui_sprite_fsm_action_t ui_sprite_chipmunk_touch_move_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg);

#ifdef __cplusplus
}
#endif

#endif

