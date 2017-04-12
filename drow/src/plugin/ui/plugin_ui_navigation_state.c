#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin/ui/plugin_ui_env_backend.h"
#include "plugin_ui_navigation_i.h"
#include "plugin_ui_phase_node_i.h"
#include "plugin_ui_state_node_i.h"

static plugin_ui_navigation_t plugin_ui_navigation_state_find_to_with_op(plugin_ui_state_t state, plugin_ui_state_t to_state, plugin_ui_navigation_state_op_t op);
static void plugin_ui_navigation_state_swap(plugin_ui_navigation_t l, plugin_ui_navigation_t r);

plugin_ui_navigation_t
plugin_ui_navigation_state_create(
    plugin_ui_state_t from, plugin_ui_state_t to,
    plugin_ui_navigation_state_op_t op, plugin_ui_renter_policy_t renter_policy)
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
    navigation->m_category = plugin_ui_navigation_category_state;
    navigation->m_renter_policy = renter_policy;
    navigation->m_trigger_control = NULL;
    navigation->m_condition = NULL;
    navigation->m_weight = 1.0f;
    
    navigation->m_from_state = from;
    TAILQ_INSERT_TAIL(&from->m_navigations_to, navigation, m_next_for_from);

    navigation->m_state.m_suspend = 0;
    navigation->m_state.m_base_policy = plugin_ui_navigation_state_base_curent;

    switch(op) {
    case plugin_ui_navigation_state_op_switch:
    case plugin_ui_navigation_state_op_call:
    case plugin_ui_navigation_state_op_template:
        assert(to);
        assert(from->m_phase == to->m_phase);

        navigation->m_to_state = to;
        TAILQ_INSERT_TAIL(&to->m_navigations_from, navigation, m_next_for_to);
        break;
    case plugin_ui_navigation_state_op_back:
        assert(to == NULL);
        navigation->m_to_state = NULL;
        break;
    default:
        CPE_ERROR(env->m_module->m_em, "plugin_ui_navigation_create: unknown op %d!", op);
        TAILQ_REMOVE(&navigation->m_from_state->m_navigations_to, navigation, m_next_for_from);
        mem_free(env->m_module->m_alloc, navigation);
    }

    navigation->m_state.m_op = op;

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

uint8_t plugin_ui_navigation_state_suspend(plugin_ui_navigation_t navigation) {
    return navigation->m_state.m_suspend;
}

void plugin_ui_navigation_state_set_suspend(plugin_ui_navigation_t navigation, uint8_t suspend) {
    navigation->m_state.m_suspend = suspend;
}

plugin_ui_state_t plugin_ui_navigation_state_to(plugin_ui_navigation_t navigation) {
    return navigation->m_to_state;
}

void plugin_ui_navigation_state_set_to(plugin_ui_navigation_t navigation, plugin_ui_state_t to) {
    if (navigation->m_to_state) {
        TAILQ_REMOVE(&navigation->m_to_state->m_navigations_from, navigation, m_next_for_to);
    }

    navigation->m_to_state = to;

    if (navigation->m_to_state) {
        TAILQ_INSERT_TAIL(&navigation->m_to_state->m_navigations_from, navigation, m_next_for_to);
    }
}

plugin_ui_navigation_state_op_t plugin_ui_navigation_state_op(plugin_ui_navigation_t navigation) {
    return navigation->m_state.m_op; 
}

const char * plugin_ui_navigation_state_op_str(plugin_ui_navigation_t navigation) {
    return plugin_ui_navigation_state_op_to_str(navigation->m_state.m_op);
}

plugin_ui_navigation_state_base_policy_t plugin_ui_navigation_state_base_policy(plugin_ui_navigation_t navigation) {
    return navigation->m_state.m_base_policy;
}

void plugin_ui_navigation_state_set_base_policy(plugin_ui_navigation_t navigation, plugin_ui_navigation_state_base_policy_t  base_policy) {
    navigation->m_state.m_base_policy = base_policy;
}

plugin_ui_state_t plugin_ui_navigation_state_loading(plugin_ui_navigation_t navigation) {
    return navigation->m_loading_state;
}

void plugin_ui_navigation_state_set_loading(plugin_ui_navigation_t navigation, plugin_ui_state_t loading_state) {
    if (navigation->m_loading_state) {
        TAILQ_REMOVE(&navigation->m_loading_state->m_navigations_as_loading, navigation, m_next_for_loading);
    }

    navigation->m_loading_state = loading_state;

    if (navigation->m_loading_state) {
        TAILQ_INSERT_TAIL(&navigation->m_loading_state->m_navigations_as_loading, navigation, m_next_for_loading);
    }
}

plugin_ui_state_t plugin_ui_navigation_state_back(plugin_ui_navigation_t navigation) {
    return navigation->m_back_state;
}

void plugin_ui_navigation_state_set_back(plugin_ui_navigation_t navigation, plugin_ui_state_t back_state) {
    if (navigation->m_back_state) {
        TAILQ_REMOVE(&navigation->m_back_state->m_navigations_as_back, navigation, m_next_for_back);
    }

    navigation->m_back_state = back_state;

    if (navigation->m_back_state) {
        TAILQ_INSERT_TAIL(&navigation->m_back_state->m_navigations_as_back, navigation, m_next_for_back);
    }
}

int plugin_ui_navigation_state_execute(plugin_ui_navigation_t navigation, plugin_ui_state_node_t state_node, dr_data_t data) {
    plugin_ui_env_t env = state_node->m_phase_node->m_env;
    plugin_ui_state_node_t result_state_node;
    uint8_t is_auto_execute = 0;
    
EXECUTE_AGAIN:
    result_state_node = NULL;

    if (plugin_ui_env_debug(env)) {
        if (navigation->m_to_state) {
            CPE_INFO(
                env->m_module->m_em, "navigation state %s %s%s to %s begin: %s",
                plugin_ui_state_name(navigation->m_from_state),
                plugin_ui_navigation_state_op_str(navigation),
                is_auto_execute ? "(auto)" : "",
                plugin_ui_state_name(navigation->m_to_state),
                plugin_ui_phase_node_dump(&env->m_module->m_dump_buffer, state_node->m_phase_node));
        }
        else {
            CPE_INFO(
                env->m_module->m_em, "navigation state %s %s%s begin: %s",
                plugin_ui_state_name(navigation->m_from_state),
                plugin_ui_navigation_state_op_str(navigation),
                is_auto_execute ? "(auto)" : "",
                plugin_ui_phase_node_dump(&env->m_module->m_dump_buffer, state_node->m_phase_node));
        }            
    }
              
    switch (navigation->m_state.m_op) {
    case plugin_ui_navigation_state_op_switch: {
        plugin_ui_state_t loading_state = navigation->m_loading_state;
        plugin_ui_state_t back_state = navigation->m_back_state;
        plugin_ui_state_node_t cur_state_node = state_node;
        
        if (cur_state_node && cur_state_node->m_from_navigation) {
            plugin_ui_navigation_t refer_navigation;
                
            if ((refer_navigation =
                 plugin_ui_navigation_state_find_to_with_op(
                     cur_state_node->m_from_navigation->m_from_state, navigation->m_to_state, plugin_ui_navigation_state_op_template)))
            {
                assert(refer_navigation->m_to_state == navigation->m_to_state);
                back_state = refer_navigation->m_back_state;
                plugin_ui_navigation_state_swap(refer_navigation, cur_state_node->m_from_navigation);
            }
            else if ((refer_navigation =
                      plugin_ui_navigation_state_find_to_with_op(
                          cur_state_node->m_from_navigation->m_from_state, navigation->m_to_state, plugin_ui_navigation_state_op_call)))
            {
                assert(refer_navigation->m_to_state == navigation->m_to_state);
                back_state = refer_navigation->m_back_state;
            }
        }

        if (plugin_ui_state_node_switch_i(state_node, navigation->m_to_state, loading_state, back_state, data) != 0) return -1;
        
        result_state_node = state_node;

        break;
    }
    case plugin_ui_navigation_state_op_call:
        result_state_node =
            plugin_ui_state_node_call_i(
                state_node,
                navigation->m_to_state, navigation->m_loading_state, navigation->m_back_state,
                navigation->m_renter_policy, navigation->m_state.m_suspend, data);
        if (result_state_node) {
            result_state_node->m_from_navigation = navigation;
        }
        break;
    case plugin_ui_navigation_state_op_back: {
        plugin_ui_state_node_back(state_node);
        break;
    }
    default:
        assert(0);
        return -1;
    }

    if (plugin_ui_env_debug(env)) {
        if (navigation->m_to_state) {
            CPE_INFO(
                env->m_module->m_em, "navigation state %s %s(%s) to %s complete: %s",
                plugin_ui_state_name(navigation->m_from_state),
                plugin_ui_navigation_state_op_str(navigation),
                is_auto_execute ? "(auto)" : "",
                plugin_ui_state_name(navigation->m_to_state),
                plugin_ui_phase_node_dump(&env->m_module->m_dump_buffer, state_node->m_phase_node));
        }
        else {
            CPE_INFO(
                env->m_module->m_em, "navigation state %s %s(%s) complete: %s",
                plugin_ui_state_name(navigation->m_from_state),
                plugin_ui_navigation_state_op_str(navigation),
                is_auto_execute ? "(auto)" : "",
                plugin_ui_phase_node_dump(&env->m_module->m_dump_buffer, state_node->m_phase_node));
        }
    }

    /*检查自动进入操作 */
    if (result_state_node && navigation->m_to_state && navigation->m_to_state->m_auto_execute) {
        state_node = result_state_node;
        navigation = navigation->m_to_state->m_auto_execute;
        is_auto_execute = 1;
        goto EXECUTE_AGAIN;
    }
    
    return 0;
}

static plugin_ui_navigation_t
plugin_ui_navigation_state_find_to_with_op(plugin_ui_state_t state, plugin_ui_state_t to_state, plugin_ui_navigation_state_op_t op) {
    plugin_ui_navigation_t navigation;

    TAILQ_FOREACH(navigation, &state->m_navigations_to, m_next_for_from) {
        if (navigation->m_to_state == to_state && navigation->m_state.m_op == op) {
            return navigation;
        }
    }
    
    return NULL;
}

static void plugin_ui_navigation_state_swap(plugin_ui_navigation_t l, plugin_ui_navigation_t r) {
    plugin_ui_renter_policy_t renter_policy = l->m_renter_policy;
    uint8_t suspend = l->m_state.m_suspend;
    plugin_ui_state_t to_state = l->m_to_state;
    plugin_ui_state_t loading_state = l->m_loading_state;
    plugin_ui_state_t back_state = l->m_back_state;
	uint8_t base_policy = l->m_state.m_base_policy;

    l->m_renter_policy = r->m_renter_policy;
    r->m_renter_policy = renter_policy;

    plugin_ui_navigation_state_set_suspend(l, r->m_state.m_suspend);
    plugin_ui_navigation_state_set_to(l, r->m_to_state);
    plugin_ui_navigation_state_set_loading(l, r->m_loading_state);
    plugin_ui_navigation_state_set_back(l, r->m_back_state);
	plugin_ui_navigation_state_set_base_policy(l, r->m_state.m_base_policy);
    
    plugin_ui_navigation_state_set_suspend(r, suspend);
    plugin_ui_navigation_state_set_to(r, to_state);
    plugin_ui_navigation_state_set_loading(r, loading_state);
    plugin_ui_navigation_state_set_back(r, back_state);
	plugin_ui_navigation_state_set_base_policy(l, base_policy);
}

const char * plugin_ui_navigation_state_op_to_str(plugin_ui_navigation_state_op_t op) {
    switch(op) {
    case plugin_ui_navigation_state_op_switch:
        return "state-switch";
    case plugin_ui_navigation_state_op_call:
        return "state-call";
    case plugin_ui_navigation_state_op_back:
        return "state-back";
    case plugin_ui_navigation_state_op_template:
        return "state-template";
    default:
        return "state-unknown";
    }
}
