#ifndef UI_SPRITE_UI_ACTION_GUARD_POPUP_I_H
#define UI_SPRITE_UI_ACTION_GUARD_POPUP_I_H
#include "ui_sprite_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_ui_action_guard_popup * ui_sprite_ui_action_guard_popup_t;
    
struct ui_sprite_ui_action_guard_popup {
    ui_sprite_ui_module_t m_module;
    uint8_t m_free_from_guard;
};

extern const char * UI_SPRITE_UI_ACTION_GUARD_POPUP_NAME;

ui_sprite_ui_action_guard_popup_t ui_sprite_ui_action_guard_popup_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_ui_action_guard_popup_free(ui_sprite_ui_action_guard_popup_t guard_popup);
    
int ui_sprite_ui_action_guard_popup_regist(ui_sprite_ui_module_t module);
void ui_sprite_ui_action_guard_popup_unregist(ui_sprite_ui_module_t module);

#ifdef __cplusplus
}
#endif

#endif
