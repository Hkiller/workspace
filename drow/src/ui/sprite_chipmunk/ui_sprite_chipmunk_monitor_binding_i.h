#ifndef UI_SPRITE_CHIPMUNK_MONITOR_BINDING_I_H
#define UI_SPRITE_CHIPMUNK_MONITOR_BINDING_I_H
#include "chipmunk/chipmunk_private.h"
#include "ui_sprite_chipmunk_obj_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_monitor_binding {
    ui_sprite_chipmunk_obj_body_t m_body;
    TAILQ_ENTRY(ui_sprite_chipmunk_monitor_binding) m_next_for_body;    
    ui_sprite_chipmunk_monitor_t m_monitor;
    TAILQ_ENTRY(ui_sprite_chipmunk_monitor_binding) m_next_for_monitor;
};

ui_sprite_chipmunk_monitor_binding_t
ui_sprite_chipmunk_monitor_binding_create(ui_sprite_chipmunk_obj_body_t body, ui_sprite_chipmunk_monitor_t monitor);

void 
ui_sprite_chipmunk_monitor_binding_free(ui_sprite_chipmunk_monitor_binding_t binding);
    
#ifdef __cplusplus
}
#endif

#endif
