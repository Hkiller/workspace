#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin_ui_control_action_i.h"
#include "plugin_ui_control_action_slots_i.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_aspect_i.h"
#include "plugin_ui_aspect_ref_i.h"

plugin_ui_control_action_t
plugin_ui_control_action_create(
    plugin_ui_control_t control, 
    plugin_ui_event_t evt, plugin_ui_event_scope_t scope,
    plugin_ui_event_fun_t fun, void * ctx)
{
    plugin_ui_control_action_t action;
    plugin_ui_env_t env = control->m_page->m_env;
    
    if (control->m_action_slots == NULL) {
        if (plugin_ui_control_action_slots_create(control) != 0) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_control_action_create: create slots fail!");
            return NULL;
        }
    }

    assert(control->m_action_slots);

    action = TAILQ_FIRST(&env->m_free_control_actions);
    if (action) {
        TAILQ_REMOVE(&env->m_free_control_actions, action, m_next);
    }
    else {
        action = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_control_action));
        if (action == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_control_action_create: alloc action fail!");
            return NULL;
        }
    }

    assert(action);

    action->m_control = control;
    TAILQ_INIT(&action->m_aspects);
    action->m_name_prefix = NULL;
    action->m_event = evt;
    action->m_scope = scope;
    action->m_fun = fun;
    action->m_ctx = ctx;

    TAILQ_INSERT_TAIL(&control->m_action_slots->m_actions[evt - plugin_ui_event_min], action, m_next);
    TAILQ_INSERT_TAIL(&control->m_page->m_control_actions, action, m_next_for_page);    

    return action;
}

void plugin_ui_control_action_free(plugin_ui_control_action_t action) {
    plugin_ui_control_t control = action->m_control;
    plugin_ui_env_t env = control->m_page->m_env;

    TAILQ_REMOVE(&control->m_action_slots->m_actions[action->m_event - plugin_ui_event_min], action, m_next);
    TAILQ_REMOVE(&control->m_page->m_control_actions, action, m_next_for_page);    

    if (action->m_name_prefix) {
        mem_free(env->m_module->m_alloc, action->m_name_prefix);
        action->m_name_prefix = NULL;
    }

    while(!TAILQ_EMPTY(&action->m_aspects)) {
        plugin_ui_aspect_ref_t ref = TAILQ_FIRST(&action->m_aspects);
        plugin_ui_aspect_ref_free(ref, &ref->m_aspect->m_control_actions, &action->m_aspects);
    }

    action->m_control = (void*)env;
    TAILQ_INSERT_TAIL(&env->m_free_control_actions, action, m_next);
}
    
void plugin_ui_control_action_real_free(plugin_ui_control_action_t action) {
    plugin_ui_env_t env = (void*)action->m_control;

    TAILQ_REMOVE(&env->m_free_control_actions, action, m_next);

    mem_free(env->m_module->m_alloc,  action);
}

uint8_t plugin_ui_control_have_action(plugin_ui_control_t control, plugin_ui_event_t evt, plugin_ui_event_scope_t scope) {
    plugin_ui_control_action_t action;

    if(control->m_action_slots == NULL) return 0;

    TAILQ_FOREACH(action, &control->m_action_slots->m_actions[evt - plugin_ui_event_min], m_next) {
        if (action->m_event == evt && action->m_scope == scope) return 1;
    }

    return 0;
}

uint8_t plugin_ui_control_action_data_capacity(plugin_ui_control_action_t action) {
    return (uint8_t)CPE_ARRAY_SIZE(action->m_data);
}

void * plugin_ui_control_action_data(plugin_ui_control_action_t action) {
    return action->m_data;
}

const char * plugin_ui_control_action_name_prefix(plugin_ui_control_action_t action) {
    return action->m_name_prefix;
}

int plugin_ui_control_action_set_name_prefix(plugin_ui_control_action_t action, const char * name_prefix) {
    plugin_ui_env_t env = action->m_control->m_page->m_env;

    if (action->m_name_prefix) {
        mem_free(env->m_module->m_alloc, action->m_name_prefix);
    }

    if (name_prefix) {
        action->m_name_prefix = cpe_str_mem_dup(env->m_module->m_alloc, name_prefix);
        if (action->m_name_prefix == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_control_action_set_name_prefix: alloc name prefix fail!");
            return -1;
        }
    }
    else {
        name_prefix = NULL;
    }

    return 0;
}

void plugin_ui_control_action_remove_in_page_by_func(plugin_ui_page_t page, plugin_ui_event_fun_t fun) {
    plugin_ui_control_action_t action, next;

    for(action = TAILQ_FIRST(&page->m_control_actions); action; action = next) {
        next = TAILQ_NEXT(action, m_next_for_page);

        if (action->m_fun == fun) plugin_ui_control_action_free(action);
    }
}
