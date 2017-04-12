#ifndef UI_SPRITE_TRI_ACTION_SEND_EVENT_I_H
#define UI_SPRITE_TRI_ACTION_SEND_EVENT_I_H
#include "ui/sprite_tri/ui_sprite_tri_action_send_event.h"
#include "ui_sprite_tri_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_tri_action_send_event {
    ui_sprite_tri_module_t m_module;
};

int ui_sprite_tri_action_send_event_regist(ui_sprite_tri_module_t module);
void ui_sprite_tri_action_send_event_unregist(ui_sprite_tri_module_t module);

#ifdef __cplusplus
}
#endif

#endif
