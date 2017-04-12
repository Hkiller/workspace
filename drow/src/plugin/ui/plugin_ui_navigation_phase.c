#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin/ui/plugin_ui_env_backend.h"
#include "plugin_ui_navigation_i.h"
#include "plugin_ui_phase_node_i.h"
#include "plugin_ui_state_node_i.h"

plugin_ui_navigation_t
plugin_ui_navigation_phase_create(
    plugin_ui_state_t from, plugin_ui_phase_t to,
    plugin_ui_navigation_phase_op_t op, plugin_ui_renter_policy_t renter_policy)
{
    plugin_ui_env_t env = from->m_phase->m_env;
    plugin_ui_navigation_t navigation;

    assert(from);

    navigation = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_navigation) + env->m_backend->navigation_capacity);
    if (navigation == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_navigation_create: alloc fail!");
        return NULL;
    }

    navigation->m_env = env;
    navigation->m_category = plugin_ui_navigation_category_phase;
    navigation->m_renter_policy = renter_policy;
    navigation->m_trigger_control = NULL;
    navigation->m_condition = NULL;
    navigation->m_weight = 1.0f;
    
    navigation->m_from_state = from;
    TAILQ_INSERT_TAIL(&from->m_navigations_to, navigation, m_next_for_from);

    switch(op) {
    case plugin_ui_navigation_phase_op_switch:
    case plugin_ui_navigation_phase_op_call:
        assert(to);
        navigation->m_to_state = to->m_init_state;
        TAILQ_INSERT_TAIL(&navigation->m_to_state->m_navigations_from, navigation, m_next_for_to);
        break;
    case plugin_ui_navigation_phase_op_back:
    case plugin_ui_navigation_phase_op_reset:
        assert(to == NULL);
        navigation->m_to_state = NULL;
        break;
    default:
        CPE_ERROR(env->m_module->m_em, "plugin_ui_navigation_create: unknown op %d!", op);
        TAILQ_REMOVE(&navigation->m_from_state->m_navigations_to, navigation, m_next_for_from);
        mem_free(env->m_module->m_alloc, navigation);
    }

    navigation->m_phase.m_op = op;

    navigation->m_loading_state = NULL;
    navigation->m_back_state = NULL;

    if (env->m_backend->navigation_init(env->m_backend->ctx, navigation) != 0) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_navigation_create: backend init fail!");

        TAILQ_REMOVE(&navigation->m_from_state->m_navigations_to, navigation, m_next_for_from);
        if (navigation->m_to_state) TAILQ_REMOVE(&navigation->m_to_state->m_navigations_from, navigation, m_next_for_to);
        
        mem_free(env->m_module->m_alloc, navigation);
        return NULL;
    }

    return navigation;
}

plugin_ui_phase_t plugin_ui_navigation_phase_to(plugin_ui_navigation_t navigation) {
    assert(navigation->m_category == plugin_ui_navigation_category_phase);
    return navigation->m_to_state->m_phase;
}

void plugin_ui_navigation_phase_set_to(plugin_ui_navigation_t navigation, plugin_ui_phase_t to) {
    assert(navigation->m_category == plugin_ui_navigation_category_phase);
    
    if (navigation->m_to_state) {
        TAILQ_REMOVE(&navigation->m_to_state->m_navigations_from, navigation, m_next_for_to);
    }

    navigation->m_to_state = to ? to->m_init_state : NULL;

    if (navigation->m_to_state) {
        TAILQ_INSERT_TAIL(&navigation->m_to_state->m_navigations_from, navigation, m_next_for_to);
    }
}

plugin_ui_navigation_phase_op_t plugin_ui_navigation_phase_op(plugin_ui_navigation_t navigation) {
    assert(navigation->m_category == plugin_ui_navigation_category_phase);
    return navigation->m_phase.m_op; 
}

const char * plugin_ui_navigation_phase_op_str(plugin_ui_navigation_t navigation) {
    assert(navigation->m_category == plugin_ui_navigation_category_phase);
    return plugin_ui_navigation_phase_op_to_str(navigation->m_phase.m_op);
}

plugin_ui_phase_t plugin_ui_navigation_phase_loading(plugin_ui_navigation_t navigation) {
    assert(navigation->m_category == plugin_ui_navigation_category_phase);
    return navigation->m_loading_state->m_phase;
}

uint8_t plugin_ui_navigation_phase_loading_auto_complete(plugin_ui_navigation_t navigation) {
    assert(navigation->m_category == plugin_ui_navigation_category_phase);
    return navigation->m_phase.m_loading_auto_complete;
}

void plugin_ui_navigation_phase_set_loading(plugin_ui_navigation_t navigation, plugin_ui_phase_t loading_phase, uint8_t auto_complete) {
    assert(navigation->m_category == plugin_ui_navigation_category_phase);
    if (navigation->m_loading_state) {
        TAILQ_REMOVE(&navigation->m_loading_state->m_navigations_as_loading, navigation, m_next_for_loading);
    }

    navigation->m_loading_state = loading_phase ? loading_phase->m_init_state : NULL;
    navigation->m_phase.m_loading_auto_complete = auto_complete;
    
    if (navigation->m_loading_state) {
        TAILQ_INSERT_TAIL(&navigation->m_loading_state->m_navigations_as_loading, navigation, m_next_for_loading);
    }
}

plugin_ui_phase_t plugin_ui_navigation_phase_back(plugin_ui_navigation_t navigation) {
    assert(navigation->m_category == plugin_ui_navigation_category_phase);
    return navigation->m_back_state->m_phase;
}

uint8_t plugin_ui_navigation_phase_back_auto_complete(plugin_ui_navigation_t navigation) {
    assert(navigation->m_category == plugin_ui_navigation_category_phase);
    return navigation->m_phase.m_back_auto_complete;
}

void plugin_ui_navigation_phase_set_back(plugin_ui_navigation_t navigation, plugin_ui_phase_t back_phase, uint8_t auto_complete) {
    assert(navigation->m_category == plugin_ui_navigation_category_phase);
    if (navigation->m_back_state) {
        TAILQ_REMOVE(&navigation->m_back_state->m_navigations_as_back, navigation, m_next_for_back);
    }

    navigation->m_back_state = back_phase ? back_phase->m_init_state : NULL;
    navigation->m_phase.m_back_auto_complete = auto_complete;
    
    if (navigation->m_back_state) {
        TAILQ_INSERT_TAIL(&navigation->m_back_state->m_navigations_as_back, navigation, m_next_for_back);
    }
}

int plugin_ui_navigation_phase_execute(plugin_ui_navigation_t navigation, plugin_ui_phase_node_t phase_node, dr_data_t data) {
    plugin_ui_env_t env = phase_node->m_env;

    if (plugin_ui_env_debug(env)) {
        if (navigation->m_to_state) {
            CPE_INFO(
                env->m_module->m_em, "navigation phase %s %s to %s begin: %s",
                plugin_ui_phase_name(navigation->m_from_state->m_phase),
                plugin_ui_navigation_phase_op_str(navigation),
                plugin_ui_phase_name(navigation->m_to_state->m_phase),
                plugin_ui_phase_node_dump(&env->m_module->m_dump_buffer, phase_node));
        }
        else {
            CPE_INFO(
                env->m_module->m_em, "navigation phase %s %s begin: %s",
                plugin_ui_phase_name(navigation->m_from_state->m_phase),
                plugin_ui_navigation_phase_op_str(navigation),
                plugin_ui_phase_node_dump(&env->m_module->m_dump_buffer, phase_node));
        }            
    }
              
    switch (navigation->m_phase.m_op) {
    case plugin_ui_navigation_phase_op_switch:
        if (plugin_ui_env_phase_switch(
                env, navigation->m_to_state->m_phase,
                navigation->m_loading_state ? navigation->m_loading_state->m_phase : NULL,
                navigation->m_phase.m_loading_auto_complete,
                data)
            != 0)
        {
            return -1;
        }
        break;
    case plugin_ui_navigation_phase_op_call:
        if (plugin_ui_env_phase_call(
                env, navigation->m_to_state->m_phase,
                navigation->m_loading_state ? navigation->m_loading_state->m_phase : NULL,
                navigation->m_phase.m_loading_auto_complete,
                navigation->m_back_state ? navigation->m_back_state->m_phase : NULL,
                navigation->m_phase.m_back_auto_complete,
                data)
            != 0)
        {
            return -1;
        }
        break;
    case plugin_ui_navigation_phase_op_back:
        if (plugin_ui_env_phase_back(env) != 0) return -1;
        break;
    case plugin_ui_navigation_phase_op_reset:
        if (plugin_ui_env_phase_reset(env) != 0) return -1;
        break;
    default:
        assert(0);
        return -1;
    }

    if (plugin_ui_env_debug(env)) {
        if (navigation->m_to_state) {
            CPE_INFO(
                env->m_module->m_em, "navigation phase %s %s to %s complete: %s",
                plugin_ui_phase_name(navigation->m_from_state->m_phase),
                plugin_ui_navigation_phase_op_str(navigation),
                plugin_ui_phase_name(navigation->m_to_state->m_phase),
                plugin_ui_phase_node_dump(&env->m_module->m_dump_buffer, phase_node));
        }
        else {
            CPE_INFO(
                env->m_module->m_em, "navigation phase %s %s complete: %s",
                plugin_ui_phase_name(navigation->m_from_state->m_phase),
                plugin_ui_navigation_phase_op_str(navigation),
                plugin_ui_phase_node_dump(&env->m_module->m_dump_buffer, phase_node));
        }
    }

    return 0;
}

const char * plugin_ui_navigation_phase_op_to_str(plugin_ui_navigation_phase_op_t op) {
    switch(op) {
    case plugin_ui_navigation_phase_op_switch:
        return "phase-switch";
    case plugin_ui_navigation_phase_op_call:
        return "phase-call";
    case plugin_ui_navigation_phase_op_back:
        return "phase-back";
    case plugin_ui_navigation_phase_op_reset:
        return "phase-reset";
    default:
        return "phase-unknown";
    }
}
