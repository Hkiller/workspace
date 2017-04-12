#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin/ui/plugin_ui_env_backend.h"
#include "plugin_ui_navigation_i.h"
#include "plugin_ui_phase_node_i.h"
#include "plugin_ui_state_node_i.h"

void plugin_ui_navigation_free(plugin_ui_navigation_t navigation) {
    plugin_ui_env_t env = navigation->m_env;

    env->m_backend->navigation_fini(env->m_backend->ctx, navigation);

    if (navigation->m_trigger_control) {
        mem_free(env->m_module->m_alloc, navigation->m_trigger_control);
        navigation->m_trigger_control = NULL;
    }
    
    if (navigation->m_condition) {
        mem_free(env->m_module->m_alloc, navigation->m_condition);
        navigation->m_condition = NULL;
    }

    if (navigation->m_from_state->m_auto_execute == navigation) {
        navigation->m_from_state->m_auto_execute = NULL;
    }

    TAILQ_REMOVE(&navigation->m_from_state->m_navigations_to, navigation, m_next_for_from);

    if (navigation->m_to_state) {
        TAILQ_REMOVE(&navigation->m_to_state->m_navigations_from, navigation, m_next_for_to);
    }

    if (navigation->m_loading_state) {
        TAILQ_REMOVE(&navigation->m_loading_state->m_navigations_as_loading, navigation, m_next_for_loading);
    }

    if (navigation->m_back_state) {
        TAILQ_REMOVE(&navigation->m_back_state->m_navigations_as_back, navigation, m_next_for_back);
    }

    mem_free(env->m_module->m_alloc, navigation);
}

plugin_ui_navigation_category_t plugin_ui_navigation_category(plugin_ui_navigation_t navigation) {
    return navigation->m_category;
}

plugin_ui_state_t plugin_ui_navigation_from(plugin_ui_navigation_t navigation) {
    return navigation->m_from_state;
}

const char * plugin_ui_navigation_trigger_control(plugin_ui_navigation_t navigation) {
    return navigation->m_trigger_control;
}

int plugin_ui_navigation_set_trigger_control(plugin_ui_navigation_t navigation, const char * control_path) {
    plugin_ui_env_t env = navigation->m_env;

    if (navigation->m_trigger_control) {
        mem_free(env->m_module->m_alloc, navigation->m_trigger_control);
    }

    if (control_path) {
        navigation->m_trigger_control = cpe_str_mem_dup(env->m_module->m_alloc, control_path);
        if (navigation->m_trigger_control == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_navigation_set_trigger: alloc fail!");
            return -1;
        }
    }
    else {
        navigation->m_trigger_control = NULL;
    }

    return 0;
}

const char * plugin_ui_navigation_condition(plugin_ui_navigation_t navigation) {
    return navigation->m_condition;
}

int plugin_ui_navigation_set_condition(plugin_ui_navigation_t navigation, const char * condition) {
    plugin_ui_env_t env = navigation->m_env;

    if (navigation->m_condition) {
        mem_free(env->m_module->m_alloc, navigation->m_condition);
    }

    if (condition) {
        navigation->m_condition = cpe_str_mem_dup(env->m_module->m_alloc, condition);
        if (navigation->m_condition == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_navigation_set_condition: alloc fail!");
            return -1;
        }
    }
    else {
        navigation->m_condition = NULL;
    }

    return 0;
}

float plugin_ui_navigation_weight(plugin_ui_navigation_t navigation) {
    return navigation->m_weight;
}

void plugin_ui_navigation_set_weight(plugin_ui_navigation_t navigation, float weight) {
    navigation->m_weight = 1.0f;
}

plugin_ui_renter_policy_t plugin_ui_renter_policy(plugin_ui_navigation_t navigation) {
    return navigation->m_renter_policy;
}

void * plugin_ui_navigation_data(plugin_ui_navigation_t navigation) {
    return navigation + 1;
}

int plugin_ui_navigation_execute(plugin_ui_navigation_t navigation, plugin_ui_state_node_t state_node, dr_data_t data) {
    if (navigation->m_category == plugin_ui_navigation_category_state) {
        switch(navigation->m_state.m_base_policy) {
        case plugin_ui_navigation_state_base_curent:
            return plugin_ui_navigation_state_execute(navigation, state_node, data);
        case plugin_ui_navigation_state_base_top:
            return plugin_ui_navigation_state_execute(navigation, TAILQ_LAST(&state_node->m_phase_node->m_state_stack, plugin_ui_state_node_list), data);
        default:
            CPE_ERROR(
                navigation->m_env->m_module->m_em, "plugin_ui_control_trigger_navigation: base policy %d unknown!",
                navigation->m_state.m_base_policy);
            return -1;
        }
    }
    else {
        return plugin_ui_navigation_phase_execute(navigation, state_node->m_phase_node, data);
    }
}
