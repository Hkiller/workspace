#ifndef UI_SPRITE_BASIC_SEND_EVENT_I_H
#define UI_SPRITE_BASIC_SEND_EVENT_I_H
#include "ui/sprite_basic/ui_sprite_basic_send_event.h"
#include "ui_sprite_basic_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_basic_send_event {
    ui_sprite_basic_module_t m_module;
    char * m_on_enter;
    char * m_on_exit;
};

int ui_sprite_basic_send_event_regist(ui_sprite_basic_module_t module);
void ui_sprite_basic_send_event_unregist(ui_sprite_basic_module_t module);

#ifdef __cplusplus
}
#endif

#endif
