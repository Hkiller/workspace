#ifndef UI_SPRITE_CHIPMUNK_SEND_EVENT_TO_COLLISION_I_H
#define UI_SPRITE_CHIPMUNK_SEND_EVENT_TO_COLLISION_I_H
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_send_event_to_collision.h"
#include "ui_sprite_chipmunk_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_send_event_to_collision {
    ui_sprite_chipmunk_module_t m_module;
    char * m_cfg_event;
    char * m_cfg_mask;
};

int ui_sprite_chipmunk_send_event_to_collision_regist(ui_sprite_chipmunk_module_t module);
void ui_sprite_chipmunk_send_event_to_collision_unregist(ui_sprite_chipmunk_module_t module);

#ifdef __cplusplus
}
#endif

#endif
