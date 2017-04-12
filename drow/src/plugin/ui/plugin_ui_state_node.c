#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "plugin/package/plugin_package_group.h"
#include "plugin/package/plugin_package_load_task.h"
#include "plugin/ui/plugin_ui_env_backend.h"
#include "plugin_ui_state_node_i.h"
#include "plugin_ui_state_node_page_i.h"

plugin_ui_state_node_t
plugin_ui_state_node_create(
    plugin_ui_phase_node_t phase_node,
    plugin_ui_state_t process_state, plugin_ui_state_t loading_state, plugin_ui_state_t back_state, uint8_t suspend_old, dr_data_t data)
{
    plugin_ui_env_t env = phase_node->m_env;
    plugin_ui_state_node_t state_node;
    plugin_ui_state_node_t pre_state_node;

    state_node = TAILQ_FIRST(&env->m_free_state_nodes);
    if (state_node) {
        TAILQ_REMOVE(&env->m_free_state_nodes, state_node, m_next);
    }
    else {
        state_node = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_state_node));
        if (state_node == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_state_node_create: alloc fail!");
            return NULL;
        }
    }

    pre_state_node = TAILQ_LAST(&phase_node->m_state_stack, plugin_ui_state_node_list);

    state_node->m_phase_node = phase_node;
    state_node->m_from_navigation = NULL;
    state_node->m_package_load_task = 0;
    state_node->m_level = pre_state_node ? (pre_state_node->m_level + 1) : 0;
    state_node->m_is_active = 0;
    state_node->m_state = plugin_ui_state_node_state_loading;
    state_node->m_op = plugin_ui_state_node_op_none;
    TAILQ_INIT(&state_node->m_pages);
    bzero(&state_node->m_curent, sizeof(state_node->m_curent));
    bzero(&state_node->m_replace, sizeof(state_node->m_replace));

    if (plugin_ui_state_node_data_init(env, &state_node->m_curent, process_state, loading_state, back_state, suspend_old, data) != 0) {
        state_node->m_curent.m_process_state = (void *)env;
        TAILQ_INSERT_TAIL(&env->m_free_state_nodes, state_node, m_next);
        return NULL;
    }

    env->m_visible_pages_need_update = 1;
    TAILQ_INSERT_TAIL(&phase_node->m_state_stack, state_node, m_next);

    return state_node;
}

void plugin_ui_state_node_free(plugin_ui_state_node_t state_node) {
    plugin_ui_env_t env = state_node->m_phase_node->m_env;

    plugin_ui_state_node_suspend(state_node);

    assert(!state_node->m_is_active);
    
    TAILQ_REMOVE(&state_node->m_phase_node->m_state_stack, state_node, m_next);

    plugin_ui_state_node_data_fini(env, &state_node->m_curent);
    plugin_ui_state_node_data_fini(env, &state_node->m_replace);

    state_node->m_curent.m_process_state = (void *)env;
    env->m_visible_pages_need_update = 1;
    TAILQ_INSERT_TAIL(&env->m_free_state_nodes, state_node, m_next);
}

void plugin_ui_state_node_real_free(plugin_ui_state_node_t state_node) {
    plugin_ui_env_t env = (void*)state_node->m_curent.m_process_state;
    
    TAILQ_REMOVE(&env->m_free_state_nodes, state_node, m_next);

    mem_free(env->m_module->m_alloc, state_node);
}

uint8_t plugin_ui_state_node_level(plugin_ui_state_node_t state_node) {
    return state_node->m_level;
}

plugin_ui_state_node_t plugin_ui_state_node_current(plugin_ui_phase_node_t phase_node) {
    plugin_ui_state_node_t state_node = TAILQ_LAST(&phase_node->m_state_stack, plugin_ui_state_node_list);

    while(state_node && (state_node->m_op == plugin_ui_state_node_op_remove
                         || state_node->m_state == plugin_ui_state_node_state_back))
    {
        state_node = TAILQ_PREV(state_node, plugin_ui_state_node_list, m_next);
    }
    
    return state_node;
}

plugin_ui_state_node_t plugin_ui_state_node_find_by_level(plugin_ui_phase_node_t phase_node, uint8_t level) {
    plugin_ui_state_node_t state_node = TAILQ_FIRST(&phase_node->m_state_stack);

    TAILQ_FOREACH(state_node, &phase_node->m_state_stack, m_next) {
        if (state_node->m_level == level) return state_node;
    }

    return NULL;
}

plugin_ui_state_node_t plugin_ui_state_node_prev(plugin_ui_state_node_t state_node) {
    return TAILQ_PREV(state_node, plugin_ui_state_node_list, m_next);
}

plugin_ui_state_node_t
plugin_ui_state_node_find_by_process(plugin_ui_phase_node_t phase_node, const char * process_state_name) {
    plugin_ui_state_node_t state_node;

    TAILQ_FOREACH_REVERSE(state_node, &phase_node->m_state_stack, plugin_ui_state_node_list, m_next) {
        if (state_node->m_op == plugin_ui_state_node_op_remove) continue;
        if (strcmp(state_node->m_curent.m_process_state->m_name, process_state_name) == 0) return state_node;
    }

    return NULL;
}

plugin_ui_state_node_t plugin_ui_state_node_find_by_current(plugin_ui_phase_node_t phase_node, const char * state_name) {
    plugin_ui_state_node_t state_node;

    TAILQ_FOREACH_REVERSE(state_node, &phase_node->m_state_stack, plugin_ui_state_node_list, m_next) {
        plugin_ui_state_t state;
        if (state_node->m_op == plugin_ui_state_node_op_remove) continue;
        
        state = plugin_ui_state_node_current_state(state_node);
        if (state && strcmp(state->m_name, state_name) == 0) return state_node;
    }

    return NULL;
}

const char * plugin_ui_state_node_name(plugin_ui_state_node_t state_node) {
    return state_node->m_curent.m_process_state->m_name;
}

const char * plugin_ui_state_node_op_str(plugin_ui_state_node_t state_node) {
    return plugin_ui_state_node_op_to_str(state_node->m_op);
}

plugin_ui_state_node_state_t plugin_ui_state_node_state(plugin_ui_state_node_t state_node) {
    return state_node->m_state;
}

const char * plugin_ui_state_node_state_str(plugin_ui_state_node_t state_node) {
    return plugin_ui_state_node_state_to_str(state_node->m_state);
}

plugin_ui_state_t plugin_ui_state_node_loading_state(plugin_ui_state_node_t state_node) {
    return state_node->m_curent.m_loading_state;
}

plugin_ui_state_t plugin_ui_state_node_process_state(plugin_ui_state_node_t state_node) {
    return state_node->m_curent.m_process_state;
}

plugin_ui_state_t plugin_ui_state_node_back_state(plugin_ui_state_node_t state_node) {
    return state_node->m_curent.m_back_state;
}

plugin_ui_state_t plugin_ui_state_node_current_state(plugin_ui_state_node_t state_node) {
    switch(state_node->m_state) {
    case plugin_ui_state_node_state_loading:
        return state_node->m_curent.m_loading_state;
    case plugin_ui_state_node_state_processing:
        return state_node->m_curent.m_process_state;
    case plugin_ui_state_node_state_back:
        return state_node->m_curent.m_back_state;
    default:
        return NULL;
    }
}

dr_data_t plugin_ui_state_node_data(plugin_ui_state_node_t state_node) {
    return state_node->m_curent.m_data.m_data ? &state_node->m_curent.m_data : NULL;
}

/*清理后续的节点，没有开始的直接删除，已经开始的设置为返回，所有replace忽略 */
static void plugin_ui_state_node_cancel_nexts(plugin_ui_state_node_t state_node) {
    plugin_ui_state_node_t next_state_node;
    
    for(next_state_node = TAILQ_NEXT(state_node, m_next); next_state_node;) {
        plugin_ui_state_node_t check_node = next_state_node;
        next_state_node = TAILQ_NEXT(next_state_node, m_next);

        plugin_ui_state_node_clear_replace(check_node);
                    
        if(check_node->m_op == plugin_ui_state_node_op_init) {
            plugin_ui_state_node_free(check_node);
        }
        else {
            check_node->m_op = plugin_ui_state_node_op_back;
        }
    }
}
    
plugin_ui_state_node_t
plugin_ui_state_node_call_i(
    plugin_ui_state_node_t from_node, plugin_ui_state_t state, plugin_ui_state_t loading_state, plugin_ui_state_t back_state, 
    plugin_ui_renter_policy_t renter_policy, uint8_t suspend_old, dr_data_t data)
{
    plugin_ui_phase_node_t cur_phase = from_node->m_phase_node;
    plugin_ui_env_t env = cur_phase->m_env;
    plugin_ui_state_node_t exist_node;
    plugin_ui_state_node_t reuse_state_node;
    plugin_ui_state_node_t new_state_node;
    
    if (env->m_debug >= 2) {
        CPE_INFO(
            env->m_module->m_em, "ui_env: %s call to %s [loading=%s, back=%s]: begin. {%s}",
            plugin_ui_state_node_name(from_node),
            state->m_name, loading_state ? loading_state->m_name : "", back_state ? back_state->m_name : "",
            plugin_ui_phase_node_dump(&env->m_module->m_dump_buffer, cur_phase));
    }

    exist_node = plugin_ui_state_node_find_by_process(from_node->m_phase_node, state->m_name);
	if (exist_node) {
        switch(renter_policy) {
        case plugin_ui_renter_skip:
            if (plugin_ui_env_debug(env)) {
                CPE_INFO(
                    env->m_module->m_em, "ui_env: %s call to %s [loading=%s, back=%s]: complete(skip). {%s}",
                    plugin_ui_state_node_name(from_node),
                    state->m_name, loading_state ? loading_state->m_name : "", back_state ? back_state->m_name : "",
                    plugin_ui_phase_node_dump(&env->m_module->m_dump_buffer, cur_phase));
            }

            return NULL;
        case plugin_ui_renter_back:
            plugin_ui_state_node_cancel_nexts(exist_node);

            if (plugin_ui_env_debug(env)) {
                CPE_INFO(
                    env->m_module->m_em, "ui_env: %s call to %s [loading=%s, back=%s]: complete(auto back). {%s}",
                    plugin_ui_state_node_name(from_node),
                    state->m_name, loading_state ? loading_state->m_name : "", back_state ? back_state->m_name : "",
                    plugin_ui_phase_node_dump(&env->m_module->m_dump_buffer, cur_phase));
            }

            return exist_node;
        case plugin_ui_renter_go:
            break;
        default:
            CPE_ERROR(
                env->m_module->m_em, "ui_env: %s call to %s [loading=%s, back=%s]: unknown renter policy %d",
                plugin_ui_state_node_name(from_node),
                state->m_name, loading_state ? loading_state->m_name : "", back_state ? back_state->m_name : "", renter_policy);
            return NULL;
        }
    }

    /*开始从from_node为基准，创建后续节点 */
    if ((reuse_state_node = TAILQ_NEXT(from_node, m_next))) {
        /*如果下一个节点存在，则尝试替换下一个节点 */
        if(reuse_state_node->m_op == plugin_ui_state_node_op_init) {
            /*下一个节点并没有开始，直接替换掉即可 */
            if (plugin_ui_state_node_data_init(
                    env, &reuse_state_node->m_curent, state, loading_state, back_state, suspend_old, data) != 0)
            {
                CPE_ERROR(
                    env->m_module->m_em, "ui_env: %s call to %s [loading=%s, back=%s], replace init state node fail",
                    plugin_ui_state_node_name(from_node),
                    state->m_name, loading_state ? loading_state->m_name : "", back_state ? back_state->m_name : "");
                return NULL;
            }
            reuse_state_node->m_op = plugin_ui_state_node_op_init;
            plugin_ui_state_node_clear_replace(reuse_state_node);

            if (env->m_debug >= 2) {
                CPE_INFO(
                    env->m_module->m_em, "ui_env: %s call to %s [loading=%s, back=%s]: complete(replace init). {%s}",
                    plugin_ui_state_node_name(from_node),
                    state->m_name, loading_state ? loading_state->m_name : "", back_state ? back_state->m_name : "",
                    plugin_ui_phase_node_dump(&env->m_module->m_dump_buffer, cur_phase));
            }
        }
        else {
            /*下一个节点已经开始，设置为返回状态，并且设置在replace */
            if (plugin_ui_state_node_data_init(env, &reuse_state_node->m_replace, state, loading_state, back_state, suspend_old, data) != 0) {
                CPE_ERROR(
                    env->m_module->m_em, "ui_env: %s call to %s [loading=%s, back=%s], set to replace state node fail",
                    plugin_ui_state_node_name(from_node),
                    state->m_name, loading_state ? loading_state->m_name : "", back_state ? back_state->m_name : "");
                return NULL;
            }
            reuse_state_node->m_op = plugin_ui_state_node_op_back;

            if (env->m_debug >= 2) {
                CPE_INFO(
                    env->m_module->m_em, "ui_env: %s call to %s [loading=%s, back=%s]: complete(replace back). {%s}",
                    plugin_ui_state_node_name(from_node),
                    state->m_name, loading_state ? loading_state->m_name : "", back_state ? back_state->m_name : "",
                    plugin_ui_phase_node_dump(&env->m_module->m_dump_buffer, cur_phase));
            }
        }

        plugin_ui_state_node_cancel_nexts(reuse_state_node);
        return reuse_state_node;
    }

    new_state_node = plugin_ui_state_node_create(from_node->m_phase_node, state, loading_state, back_state, suspend_old, data);
    if (new_state_node == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_env: %s call to %s [loading=%s, back=%s], create state nodes fail",
            plugin_ui_state_node_name(from_node),
            state->m_name, loading_state ? loading_state->m_name : "", back_state ? back_state->m_name : "");
        return NULL;
    }
    new_state_node->m_op = plugin_ui_state_node_op_init;
    
    if (env->m_debug >= 2) {
        CPE_INFO(
            env->m_module->m_em, "ui_env: %s call to %s [loading=%s, back=%s]: begin(new state)",
            plugin_ui_state_node_name(from_node),
            state->m_name, loading_state ? loading_state->m_name : "", back_state ? back_state->m_name : "");
    }

    return new_state_node;
}

plugin_ui_state_node_t
plugin_ui_state_node_call(
    plugin_ui_state_node_t from_node, const char * state_name, const char * loading_state_name, const char * back_state_name,
    plugin_ui_renter_policy_t renter_policy, uint8_t suspend_old, dr_data_t data)
{
    plugin_ui_env_t env = from_node->m_phase_node->m_env;
    plugin_ui_phase_t phase = plugin_ui_phase_node_current_phase(from_node->m_phase_node);
    plugin_ui_state_t state, loading_state, back_state;

    if (phase == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_env: %s call to %s [loading=%s, back=%s]: curent phase is not runing",
            plugin_ui_state_node_name(from_node),
            state_name, loading_state_name ? loading_state_name : "", back_state_name ? back_state_name : "");
        return NULL;
    }

    state = plugin_ui_state_find(phase, state_name);
    if (state == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_env: %s call to %s [loading=%s, back=%s]: state not exist",
            plugin_ui_state_node_name(from_node),
            state_name, loading_state_name ? loading_state_name : "", back_state_name ? back_state_name : "");
        return NULL;
    }

    loading_state = NULL;
    if (loading_state_name) {
        loading_state = plugin_ui_state_find(phase, loading_state_name);
        if (loading_state == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "ui_env: %s call to %s [loading=%s, back=%s]: loading state not exist",
                plugin_ui_state_node_name(from_node),
                state_name, loading_state_name ? loading_state_name : "", back_state_name ? back_state_name : "");
            return NULL;
        }
    }

    back_state = NULL;
    if (back_state_name) {
        back_state = plugin_ui_state_find(phase, back_state_name);
        if (back_state == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "ui_env: %s call to %s [loading=%s, back=%s]: back state not exist",
                plugin_ui_state_node_name(from_node),
                state_name, loading_state_name ? loading_state_name : "", back_state_name ? back_state_name : "");
            return NULL;
        }
    }

    return plugin_ui_state_node_call_i(from_node, state, loading_state, back_state, renter_policy, suspend_old, data);
}

int plugin_ui_state_node_reset(plugin_ui_phase_node_t phase_node) {
    plugin_ui_env_t env = phase_node->m_env;
    plugin_ui_state_node_t state_node;

    TAILQ_FOREACH(state_node, &phase_node->m_state_stack, m_next) {
        state_node->m_op = plugin_ui_state_node_op_remove;
    }

    if (env->m_debug) {
        CPE_INFO(env->m_module->m_em, "plugin_ui_state_node_reset: begin");
    }
    
    return 0;
}

int plugin_ui_state_node_switch_i(
    plugin_ui_state_node_t from_node, plugin_ui_state_t state, plugin_ui_state_t loading_state, plugin_ui_state_t back_state, dr_data_t data) 
{
    plugin_ui_env_t env = from_node->m_phase_node->m_env;
    plugin_ui_state_node_t follow_node;
    
    if (plugin_ui_state_node_data_init(
            env, &from_node->m_replace, state, loading_state, back_state, from_node->m_curent.m_suspend_old, data) != 0)
    {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_state_node_switch: switch to %s [loading=%s, back=%s], set node data fail!",
            state->m_name, loading_state ? loading_state->m_name : "", back_state ? back_state->m_name : "");
        return -1;
    }
    from_node->m_op = plugin_ui_state_node_op_switch;

    for(follow_node = TAILQ_NEXT(from_node, m_next); follow_node; follow_node = TAILQ_NEXT(follow_node, m_next)) {
        follow_node->m_op = plugin_ui_state_node_op_back;
        plugin_ui_state_node_data_fini(env, &follow_node->m_replace);
    }
    
    if (env->m_debug) {
        CPE_INFO(
            env->m_module->m_em, "plugin_ui_state_node_switch: switch to %s [loading=%s, back=%s]: begin",
            state->m_name, loading_state ? loading_state->m_name : "", back_state ? back_state->m_name : "");
    }

    return 0;
}

int plugin_ui_state_node_switch(
    plugin_ui_state_node_t from_node, const char * state_name, const char * loading_state_name, const char * back_state_name, dr_data_t data)
{
    plugin_ui_env_t env = from_node->m_phase_node->m_env;
    plugin_ui_phase_t phase = plugin_ui_phase_node_current_phase(from_node->m_phase_node);
    plugin_ui_state_t state, loading_state, back_state;
    
    state = plugin_ui_state_find(phase, state_name);
    if (state == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_state_node_switch: switch to %s [loading=%s, back=%s]: state not exist",
            state_name, loading_state_name ? loading_state_name : "", back_state_name ? back_state_name : "");
        return -1;
    }

    loading_state = NULL;
    if (loading_state_name) {
        loading_state = plugin_ui_state_find(phase, loading_state_name);
        if (loading_state == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_state_node_switch: switch to %s [loading=%s, back=%s]: loading state not exist",
                state_name, loading_state_name ? loading_state_name : "", back_state_name ? back_state_name : "");
            return -1;
        }
    }

    back_state = NULL;
    if (back_state_name) {
        back_state = plugin_ui_state_find(phase, back_state_name);
        if (back_state == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_state_node_switch: switch to %s [loading=%s, back=%s]: back state not exist",
                state_name, loading_state_name ? loading_state_name : "", back_state_name ? back_state_name : "");
            return -1;
        }
    }
    
    return plugin_ui_state_node_switch_i(from_node, state, loading_state, back_state, data);
}

void plugin_ui_state_node_back(plugin_ui_state_node_t state_node) {
    plugin_ui_env_t env = state_node->m_phase_node->m_env;
    plugin_ui_state_node_t next_state_node;
    
    state_node->m_op = plugin_ui_state_node_op_back;
    plugin_ui_state_node_data_fini(env, &state_node->m_replace);

    for(next_state_node = TAILQ_NEXT(state_node, m_next); next_state_node; next_state_node = TAILQ_NEXT(next_state_node, m_next)) {
        next_state_node->m_op = plugin_ui_state_node_op_back;
        plugin_ui_state_node_data_fini(env, &next_state_node->m_replace);
    }
    
    if (env->m_debug) {
        CPE_INFO(env->m_module->m_em, "plugin_ui_state_node_back: node %s begin back", state_node->m_curent.m_process_state->m_name);
    }
}

void plugin_ui_state_node_remove(plugin_ui_state_node_t state_node) {
    plugin_ui_env_t env = state_node->m_phase_node->m_env;

    for(; state_node; state_node = TAILQ_NEXT(state_node, m_next)) {
        state_node->m_op = plugin_ui_state_node_op_remove;
    }

    if (env->m_debug) {
        CPE_INFO(env->m_module->m_em, "plugin_ui_state_node_remove: begin");
    }
}

void plugin_ui_state_node_suspend(plugin_ui_state_node_t state_node) {
    plugin_ui_env_t env = state_node->m_phase_node->m_env;

    plugin_ui_state_node_stop_package_load_task(state_node);
    
    if (state_node->m_is_active) {
        env->m_backend->state_node_deactive(env->m_backend->ctx, state_node);
        state_node->m_is_active = 0;
    }

    while(!TAILQ_EMPTY(&state_node->m_pages)) {
        plugin_ui_state_node_page_free(TAILQ_FIRST(&state_node->m_pages));
    }
}

uint8_t plugin_ui_state_node_is_active(plugin_ui_state_node_t state_node) {
    plugin_ui_env_t env = state_node->m_phase_node->m_env;
    
    if (!state_node->m_is_active) return 0;
    
    return env->m_backend->state_node_is_active(env->m_backend->ctx, state_node);
}

int plugin_ui_state_node_active(plugin_ui_state_node_t state_node, plugin_ui_state_node_state_t next_state) {
    plugin_ui_env_t env = state_node->m_phase_node->m_env;

    assert(!state_node->m_is_active);
    
    if (env->m_debug) {
        CPE_INFO(
            env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s: state %s ==> %s",
            env->m_tick, plugin_ui_phase_node_name(state_node->m_phase_node),
            plugin_ui_state_node_name(state_node), plugin_ui_state_node_state_to_str(next_state));
    }
    
    switch(next_state) {
    case plugin_ui_state_node_state_loading:
        assert(state_node->m_curent.m_loading_state);
        state_node->m_state = next_state;
        if (env->m_backend->state_node_active(env->m_backend->ctx, state_node, state_node->m_curent.m_loading_state) != 0) {
            return -1;
        }
        break;
    case plugin_ui_state_node_state_processing:        
        assert(state_node->m_curent.m_process_state);
        state_node->m_state = next_state;
        if (env->m_backend->state_node_active(env->m_backend->ctx, state_node, state_node->m_curent.m_process_state) != 0) {
            return -1;
        }
        break;
    case plugin_ui_state_node_state_back:
        assert(state_node->m_curent.m_back_state);
        state_node->m_state = next_state;
        if (env->m_backend->state_node_active(env->m_backend->ctx, state_node, state_node->m_curent.m_back_state) != 0) {
            return -1;
        }
        break;
    default:
        assert(0);
        break;
    }

    state_node->m_is_active = 1;
    env->m_visible_pages_need_update = 1;
    
    return 0;
}

void plugin_ui_state_node_clear_replace(plugin_ui_state_node_t node) {
    bzero(&node->m_replace, sizeof(node->m_replace));
}

void plugin_ui_state_node_replace(plugin_ui_state_node_t node, uint8_t force_replace_data) {
    plugin_ui_env_t env = node->m_phase_node->m_env;

    assert(node->m_replace.m_process_state);

    if (node->m_curent.m_inout_packages) {
        plugin_package_group_free(node->m_curent.m_inout_packages);
        env->m_package_need_gc = 1;
    }
    node->m_curent.m_inout_packages = node->m_replace.m_inout_packages;
    node->m_replace.m_inout_packages = NULL;

    if (node->m_curent.m_runtime_packages) {
        plugin_package_group_free(node->m_curent.m_runtime_packages);
        env->m_package_need_gc = 1;
    }
    node->m_curent.m_runtime_packages = node->m_replace.m_runtime_packages;
    node->m_replace.m_runtime_packages = NULL;
    
    node->m_curent.m_process_state = node->m_replace.m_process_state;
    node->m_curent.m_loading_state = node->m_replace.m_loading_state;
    node->m_curent.m_back_state = node->m_replace.m_back_state;
    node->m_curent.m_suspend_old = node->m_replace.m_suspend_old;

    if (node->m_replace.m_data.m_data) {
        if (node->m_curent.m_data.m_data && node->m_curent.m_data.m_data != node->m_curent.m_data_inline_buf) {
            mem_free(env->m_module->m_alloc, node->m_curent.m_data.m_data);
        }
                
        if (node->m_replace.m_data.m_data == node->m_replace.m_data_inline_buf) {
            node->m_curent.m_data.m_size = node->m_replace.m_data.m_size;
            node->m_curent.m_data.m_meta = node->m_replace.m_data.m_meta;
            node->m_curent.m_data.m_data = node->m_curent.m_data_inline_buf;
            memcpy(node->m_curent.m_data_inline_buf, node->m_replace.m_data_inline_buf, node->m_replace.m_data.m_size);
        }
        else {
            node->m_curent.m_data = node->m_replace.m_data;
        }
    }
    else if (force_replace_data) {
        if (node->m_curent.m_data.m_data && node->m_curent.m_data.m_data != node->m_curent.m_data_inline_buf) {
            mem_free(env->m_module->m_alloc, node->m_curent.m_data.m_data);
        }

        node->m_curent.m_data.m_size = 0;
        node->m_curent.m_data.m_meta = NULL;
        node->m_curent.m_data.m_data = NULL;
    }

    plugin_ui_state_node_clear_replace(node);
}

int plugin_ui_state_node_data_init(
    plugin_ui_env_t env, plugin_ui_state_node_data_t node_data,
    plugin_ui_state_t process_state, plugin_ui_state_t loading_state, plugin_ui_state_t back_state,
    uint8_t suspend_old, dr_data_t data)
{
    assert(process_state);
    
    node_data->m_process_state = process_state;
    node_data->m_loading_state = loading_state;
    node_data->m_back_state = back_state;
    node_data->m_suspend_old = suspend_old;
    node_data->m_inout_packages = env->m_next_state_loading_packages;
    node_data->m_runtime_packages = env->m_next_state_runing_packages;

    env->m_next_state_loading_packages = NULL;
    env->m_next_state_runing_packages = NULL;

    if (node_data->m_inout_packages) {
        char buf[64];
        snprintf(buf, sizeof(buf), "state.%s.inout", plugin_ui_state_name(process_state));
        plugin_package_group_set_name(node_data->m_inout_packages, buf);
    }

    if (node_data->m_runtime_packages) {
        char buf[64];
        snprintf(buf, sizeof(buf), "state.%s.runing", plugin_ui_state_name(process_state));
        plugin_package_group_set_name(node_data->m_runtime_packages, buf);
    }
    
    if (node_data->m_data.m_data && node_data->m_data.m_data != node_data->m_data_inline_buf) {
        mem_free(env->m_module->m_alloc, node_data->m_data.m_data);
    }
    
    if (data) {
        node_data->m_data.m_meta = data->m_meta;
        node_data->m_data.m_size = data->m_size;
        if (data->m_size <= CPE_ARRAY_SIZE(node_data->m_data_inline_buf)) {
            node_data->m_data.m_data = node_data->m_data_inline_buf;
        }
        else {
            node_data->m_data.m_data = mem_alloc(env->m_module->m_alloc, data->m_size);
            if (node_data->m_data.m_data == NULL) {
                CPE_ERROR(env->m_module->m_em, "plugin_ui_state_node_data_init: malloc data buf fail!");
                return -1;
            }
        }
        memcpy(node_data->m_data.m_data, data->m_data, data->m_size);
    }
    else {
        node_data->m_data.m_meta = NULL;
        node_data->m_data.m_size = 0;
        node_data->m_data.m_data = NULL;
    }
    
    return 0;
}

void plugin_ui_state_node_data_fini(plugin_ui_env_t env, plugin_ui_state_node_data_t node_data) {
    if (node_data->m_inout_packages) {
        plugin_package_group_free(node_data->m_inout_packages);
        node_data->m_inout_packages = NULL;
    }
    
    if (node_data->m_runtime_packages) {
        plugin_package_group_free(node_data->m_runtime_packages);
        node_data->m_runtime_packages = NULL;
    }
    
    if (node_data->m_data.m_data && node_data->m_data.m_data != node_data->m_data_inline_buf) {
        mem_free(env->m_module->m_alloc, node_data->m_data.m_data);
    }
    
    node_data->m_data.m_data = NULL;
    node_data->m_data.m_size = 0;
    node_data->m_data.m_meta = NULL;
    
    node_data->m_process_state = NULL;
    node_data->m_loading_state = NULL;
    node_data->m_back_state = NULL;
}

static plugin_ui_state_node_t plugin_ui_phae_node_state_node_next(struct plugin_ui_state_node_it * it) {
    plugin_ui_state_node_t * data = (plugin_ui_state_node_t *)(it->m_data);
    plugin_ui_state_node_t r;
    if (*data == NULL) return NULL;
    r = *data;
    *data = TAILQ_NEXT(r, m_next);
    return r;
}

void plugin_ui_phase_node_state_nodes(plugin_ui_phase_node_t phase_node, plugin_ui_state_node_it_t state_node_it) {
    *(plugin_ui_state_node_t *)(state_node_it->m_data) = TAILQ_FIRST(&phase_node->m_state_stack);
    state_node_it->next = plugin_ui_phae_node_state_node_next;
}

const char * plugin_ui_state_node_op_to_str(enum plugin_ui_state_node_op op) {
    switch(op) {
    case plugin_ui_state_node_op_none:
        return "none";
    case plugin_ui_state_node_op_init:
        return "init";
    case plugin_ui_state_node_op_remove:
        return "remove"; 
    case plugin_ui_state_node_op_back:
        return "back";
    case plugin_ui_state_node_op_switch:
        return "switch";
    default:
        return "unknown";
    }
}

const char * plugin_ui_state_node_state_to_str(plugin_ui_state_node_state_t state_node_state) {
    switch(state_node_state) {
    case plugin_ui_state_node_state_prepare_loading:
        return "prepare-loading";
    case plugin_ui_state_node_state_loading:
        return "loading";
    case plugin_ui_state_node_state_prepare_back:
        return "prepare-back";
    case plugin_ui_state_node_state_back:
        return "back";
    case plugin_ui_state_node_state_suspend:
        return "suspend";
    case plugin_ui_state_node_state_prepare_resume:
        return "prepare-resume";
    case plugin_ui_state_node_state_processing:
        return "processing";
    default:
        return "unknown";
    }
}

void plugin_ui_state_node_stop_package_load_task(plugin_ui_state_node_t state_node) {
    if (state_node->m_package_load_task) {
        plugin_package_load_task_t task =
            plugin_package_load_task_find_by_id(
            state_node->m_phase_node->m_env->m_module->m_package_module, state_node->m_package_load_task);
        if (task) {
            plugin_package_load_task_free(task);
        }
        state_node->m_package_load_task = 0;
    }
}

uint8_t plugin_ui_state_node_check_package_load_task_runing(plugin_ui_state_node_t state_node) {
    plugin_package_load_task_t task;
    
    if (state_node->m_package_load_task == 0) return 0;
    
    task = plugin_package_load_task_find_by_id(
        state_node->m_phase_node->m_env->m_module->m_package_module, state_node->m_package_load_task);
    if (task == NULL) {
        state_node->m_package_load_task = 0;
        return 0;
    }
    else {
        return 1;
    }
}

plugin_package_load_task_t
plugin_ui_state_node_create_package_load_task(plugin_ui_state_node_t state_node) {
    plugin_package_load_task_t task;

    assert(state_node->m_package_load_task == 0);

    task = plugin_package_load_task_create(
        state_node->m_phase_node->m_env->m_module->m_package_module, state_node, NULL, 0);
    if (task == NULL) {
        CPE_ERROR(state_node->m_phase_node->m_env->m_module->m_em, "plugin_ui_state_node_create_package_load_task: create fail!");
        return NULL;
    }

    state_node->m_package_load_task = plugin_package_load_task_id(task);
    return task;
}

plugin_package_group_t
plugin_ui_state_node_inout_packages_check_create(plugin_ui_state_node_t state_node) {
    char buf[64];
    snprintf(buf, sizeof(buf), "state.%s.inout", plugin_ui_state_node_name(state_node));
    state_node->m_curent.m_inout_packages = plugin_package_group_create(state_node->m_phase_node->m_env->m_module->m_package_module, buf);
    if (state_node->m_curent.m_inout_packages == NULL) {
        CPE_ERROR(
            state_node->m_phase_node->m_env->m_module->m_em,
            "plugin_ui_state_node_inout_packages_check_create: create inout packages fail!");
        return NULL;
    }
    return state_node->m_curent.m_inout_packages;
}

plugin_package_group_t
plugin_ui_state_node_runtime_packages_check_create(plugin_ui_state_node_t state_node) {
    char buf[64];
    snprintf(buf, sizeof(buf), "state.%s.runtime", plugin_ui_state_node_name(state_node));
    state_node->m_curent.m_runtime_packages = plugin_package_group_create(state_node->m_phase_node->m_env->m_module->m_package_module, buf);
    if (state_node->m_curent.m_runtime_packages == NULL) {
        CPE_ERROR(
            state_node->m_phase_node->m_env->m_module->m_em,
            "plugin_ui_state_node_runtime_packages_check_create: create runtime packages fail!");
        return NULL;
    }
    return state_node->m_curent.m_runtime_packages; 
}
