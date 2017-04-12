#include "ui/sprite/ui_sprite_entity_attr.h"
#include "ui_sprite_fsm_ins_attr_binding_i.h"

ui_sprite_fsm_action_attr_binding_t
ui_sprite_fsm_action_attr_binding_create(ui_sprite_fsm_action_t action, ui_sprite_attr_monitor_t handler) {
    ui_sprite_fsm_module_t module = action->m_state->m_ins->m_module;

    ui_sprite_fsm_action_attr_binding_t attr_binding = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_fsm_action_attr_binding));
    if (attr_binding == NULL) {
        CPE_ERROR(module->m_em, "action_attr_binding create fail!");
        return NULL;
    }

    attr_binding->m_handler = handler;
    TAILQ_INSERT_TAIL(&action->m_attr_bindings, attr_binding, m_next);

    return attr_binding;
}

void ui_sprite_fsm_action_attr_binding_free(ui_sprite_fsm_action_t action, ui_sprite_fsm_action_attr_binding_t attr_binding) {
    ui_sprite_fsm_module_t module = action->m_state->m_ins->m_module;

    ui_sprite_attr_monitor_free(ui_sprite_fsm_action_to_world(action), attr_binding->m_handler);

    TAILQ_REMOVE(&action->m_attr_bindings, attr_binding, m_next);

    mem_free(module->m_alloc, attr_binding);
}

void ui_sprite_fsm_action_attr_binding_free_all(ui_sprite_fsm_action_t action) {
    while(!TAILQ_EMPTY(&action->m_attr_bindings)) {
        ui_sprite_fsm_action_attr_binding_free(action, TAILQ_FIRST(&action->m_attr_bindings));
    }
}
