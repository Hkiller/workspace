#include <assert.h>
#include "plugin_ui_control_action_slots_i.h"
#include "plugin_ui_control_action_i.h"
#include "plugin_ui_page_i.h"

int plugin_ui_control_action_slots_create(plugin_ui_control_t control) {
    plugin_ui_control_action_slots_t slots;
    uint8_t i;

    assert(control->m_action_slots == NULL);

    if (control->m_action_slots) {
        slots = control->m_action_slots;
        control->m_action_slots = slots->m_next;
    }
    else {
        slots = mem_alloc(control->m_page->m_env->m_module->m_alloc, sizeof(struct plugin_ui_control_action_slots));
        if (slots == NULL) {
            CPE_ERROR(control->m_page->m_env->m_module->m_em, "plugin_ui_control_action_slots_create: alloc fail!");
            return -1;
        }
    }

    for(i = 0; i < CPE_ARRAY_SIZE(slots->m_actions); ++i) {
        TAILQ_INIT(&slots->m_actions[i]);
    }

    control->m_action_slots = slots;

    return 0;
}

void plugin_ui_control_action_slots_free(plugin_ui_control_t control, plugin_ui_control_action_slots_t slots) {
    plugin_ui_env_t env = control->m_page->m_env;
    uint8_t i;

    assert(control->m_action_slots == slots);
    
    for(i = 0; i < CPE_ARRAY_SIZE(slots->m_actions); ++i) {
        while(!TAILQ_EMPTY(&slots->m_actions[i])) {
            plugin_ui_control_action_free(TAILQ_FIRST(&slots->m_actions[i]));
        }
    }

    slots->m_next = env->m_free_control_action_slots;
    env->m_free_control_action_slots = slots;
}

void plugin_ui_control_action_slots_real_free(plugin_ui_env_t env, plugin_ui_control_action_slots_t slots) {
    assert(env->m_free_control_action_slots == slots);

    env->m_free_control_action_slots = slots->m_next;

    mem_free(env->m_module->m_alloc, slots);
}
