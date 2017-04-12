#include "ui/sprite/ui_sprite_event.h"
#include "ui_sprite_fsm_ins_event_binding_i.h"

ui_sprite_fsm_action_event_binding_t
ui_sprite_fsm_action_event_binding_create(ui_sprite_fsm_action_t action, ui_sprite_event_handler_t handler) {
    ui_sprite_fsm_module_t module = action->m_state->m_ins->m_module;

    ui_sprite_fsm_action_event_binding_t event_binding = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_fsm_action_event_binding));
    if (event_binding == NULL) {
        CPE_ERROR(module->m_em, "action_event_binding create fail!");
        return NULL;
    }

    event_binding->m_handler = handler;
    TAILQ_INSERT_TAIL(&action->m_event_bindings, event_binding, m_next);

    return event_binding;
}

void ui_sprite_fsm_action_event_binding_free(ui_sprite_fsm_action_t action, ui_sprite_fsm_action_event_binding_t event_binding) {
    ui_sprite_fsm_module_t module = action->m_state->m_ins->m_module;

    ui_sprite_event_handler_free(ui_sprite_fsm_action_to_world(action), event_binding->m_handler);

    TAILQ_REMOVE(&action->m_event_bindings, event_binding, m_next);

    mem_free(module->m_alloc, event_binding);
}

void ui_sprite_fsm_action_event_binding_free_all(ui_sprite_fsm_action_t action) {
    while(!TAILQ_EMPTY(&action->m_event_bindings)) {
        ui_sprite_fsm_action_event_binding_free(action, TAILQ_FIRST(&action->m_event_bindings));
    }
}
