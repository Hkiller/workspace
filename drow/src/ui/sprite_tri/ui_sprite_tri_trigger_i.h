#ifndef UI_SPRITE_TRI_TRIGGER_I_H
#define UI_SPRITE_TRI_TRIGGER_I_H
#include "ui/sprite_tri/ui_sprite_tri_trigger.h"
#include "ui_sprite_tri_rule_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_tri_trigger {
    ui_sprite_tri_rule_t m_rule;
    TAILQ_ENTRY(ui_sprite_tri_trigger) m_next;
    ui_sprite_tri_trigger_type_t m_type;
    union {
        struct {
            ui_sprite_event_handler_t m_handler;
            char * m_condition;
        } m_event;
        struct {
            ui_sprite_attr_monitor_t m_monitor;
            char * m_condition;
        } m_attr;
    };
};

void ui_sprite_tri_trigger_real_free(ui_sprite_tri_trigger_t trigger);

#ifdef __cplusplus
}
#endif

#endif
