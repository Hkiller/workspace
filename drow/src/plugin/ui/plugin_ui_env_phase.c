#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_stdio.h"
#include "render/cache/ui_cache_res.h"
#include "render/cache/ui_cache_group.h"
#include "render/cache/ui_cache_manager.h"
#include "render/model/ui_data_src_group.h"
#include "render/runtime/ui_runtime_module.h"
#include "plugin/package/plugin_package_module.h"
#include "plugin/package/plugin_package_package.h"
#include "plugin/package/plugin_package_queue.h"
#include "plugin/package/plugin_package_group.h"
#include "plugin/package/plugin_package_load_task.h"
#include "plugin_ui_env_i.h"
#include "plugin_ui_package_queue_managed_i.h"
#include "plugin_ui_package_queue_using_i.h"
#include "plugin_ui_phase_i.h"
#include "plugin_ui_phase_node_i.h"
#include "plugin_ui_state_node_i.h"
#include "plugin_ui_aspect_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_popup_i.h"

static void plugin_ui_env_process_phase_ops(plugin_ui_env_t env);
static int plugin_ui_env_sync_pages_and_packages(
    plugin_ui_env_t env, plugin_ui_phase_node_t phase_node, plugin_ui_phase_t inout, plugin_ui_phase_t runing);

static uint8_t plugin_ui_env_update_phase_do_prepare_loading(plugin_ui_env_t env, plugin_ui_phase_node_t top_node);
static uint8_t plugin_ui_env_update_phase_do_loading(plugin_ui_env_t env, plugin_ui_phase_node_t top_node);
static uint8_t plugin_ui_env_update_phase_do_prepare_back(plugin_ui_env_t env, plugin_ui_phase_node_t top_node);
static uint8_t plugin_ui_env_update_phase_do_back(plugin_ui_env_t env, plugin_ui_phase_node_t top_node);
static uint8_t plugin_ui_env_update_phase_do_prepare_resume(plugin_ui_env_t env, plugin_ui_phase_node_t top_node);

void plugin_ui_env_update_phase(plugin_ui_env_t env) {
    plugin_ui_phase_node_t top_node;
    uint8_t is_top_node_inited = 0;
    
UPDATE_TRY_AGAIN:    
    top_node = TAILQ_LAST(&env->m_phase_stack, plugin_ui_phase_node_list);

    if (top_node == NULL) {
        if (is_top_node_inited) return;
        is_top_node_inited = 1;
        
        if (env->m_init_phase == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_env_update_phase: no init phase!");
            return;
        }
        
        top_node = plugin_ui_phase_node_create(
            env, env->m_init_phase, env->m_init_call_phase, env->m_init_call_phase_auto_complete, NULL, 0, NULL);
        if (top_node == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_env_update_phase: create first phase node fail!");
            return;
        }
        top_node->m_op = plugin_ui_phase_node_op_init;
    }

    if (top_node->m_op != plugin_ui_phase_node_op_none) {
        plugin_ui_env_process_phase_ops(env);
        goto UPDATE_TRY_AGAIN;
    }
    
    switch(top_node->m_state) {
    case plugin_ui_phase_node_state_prepare_loading:
        /*初始化状态需要检查前置阶段的操作 */
        if (plugin_ui_phase_node_check_package_load_task_runing(top_node)) {
            CPE_INFO(env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s: init: still waiting", env->m_tick, plugin_ui_phase_node_name(top_node));
            return;
        }

        if (plugin_ui_env_update_phase_do_prepare_loading(env, top_node)) goto UPDATE_TRY_AGAIN;
        break;
    case plugin_ui_phase_node_state_loading:
        /*加载未完成 */
        if (plugin_package_module_total_load_complete_count(env->m_module->m_package_module)
            != plugin_package_module_total_load_count(env->m_module->m_package_module))
        {
            plugin_ui_env_update_state(top_node, 0);
            break;
        }

        assert(plugin_ui_phase_node_check_package_load_task_runing(top_node) == 0);
        if (plugin_ui_env_update_phase_do_loading(env, top_node)) goto UPDATE_TRY_AGAIN;
        break;
    case plugin_ui_phase_node_state_prepare_back:
        if (plugin_ui_phase_node_check_package_load_task_runing(top_node)) {
            CPE_INFO(env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s: prepare back: still waiting", env->m_tick, plugin_ui_phase_node_name(top_node));
            return;
        }

        if (plugin_ui_env_update_phase_do_prepare_back(env, top_node)) goto UPDATE_TRY_AGAIN;
        break;
    case plugin_ui_phase_node_state_back:
        /*加载未完成 */
        if (plugin_package_module_total_load_complete_count(env->m_module->m_package_module)
             != plugin_package_module_total_load_count(env->m_module->m_package_module))
        {
            plugin_ui_env_update_state(top_node, 0);
            break;
        }

        if (plugin_ui_env_update_phase_do_back(env, top_node)) goto UPDATE_TRY_AGAIN;
        break;
    case plugin_ui_phase_node_state_prepare_resume:
        if (plugin_ui_phase_node_check_package_load_task_runing(top_node)) {
            CPE_INFO(env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s: suspend: still waiting", env->m_tick, plugin_ui_phase_node_name(top_node));
            return;
        }

        if (plugin_ui_env_update_phase_do_prepare_resume(env, top_node)) goto UPDATE_TRY_AGAIN;
        break;
    case plugin_ui_phase_node_state_processing:
        plugin_ui_env_update_state(top_node, 1);
        if (env->m_package_need_gc) plugin_ui_env_package_gc(env);
        break;
    case plugin_ui_phase_node_state_suspend:
        CPE_INFO(env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s: update in state suspend, auto resume", env->m_tick, plugin_ui_phase_node_name(top_node));
        top_node->m_op = plugin_ui_phase_node_op_resume;
        goto UPDATE_TRY_AGAIN;
    default:
        assert(0);
    }
}

static uint8_t plugin_ui_env_update_phase_do_prepare_loading(plugin_ui_env_t env, plugin_ui_phase_node_t top_node) {
    if (top_node->m_loading_phase) {
        /*加载运行时包 */
        plugin_package_group_load_async(top_node->m_runtime_packages, NULL);
        
        /*加载锁定包 */
        if (env->m_lock_packages == NULL) {
            plugin_ui_env_init_lock_packages(env);
            if (env->m_lock_packages) {
                plugin_package_group_load_async(env->m_lock_packages, NULL);
            }
        }
        
        if (plugin_ui_phase_enter(top_node->m_loading_phase) != 0) {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_env_update_phase: enter loading phase %s (from init) fail, skip!",
                plugin_ui_phase_name(top_node->m_loading_phase));
            plugin_ui_phase_node_free(top_node);
            return 1;
        }
        else {
            top_node->m_state = plugin_ui_phase_node_state_loading;
            if (env->m_debug) {
                CPE_INFO(env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s ==> %s", env->m_tick, plugin_ui_phase_node_name(top_node), plugin_ui_phase_node_state_str(top_node));
            }
            
            plugin_ui_env_update_state(top_node, 1);
        }
    }
    else {
        plugin_package_group_clear(top_node->m_inout_packages);
        if (plugin_ui_phase_enter(top_node->m_process_phase) != 0) {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_env_update_phase: enter process phase %s (from init) fail!",
                plugin_ui_phase_name(top_node->m_process_phase));
            plugin_ui_phase_node_free(top_node);
            return 1;
        }
        else {
            top_node->m_state = plugin_ui_phase_node_state_processing;
            if (env->m_debug) {
                CPE_INFO(env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s do process", env->m_tick, plugin_ui_phase_node_name(top_node));
            }
            
            plugin_ui_env_update_state(top_node, 1);
        }
    }

    return 1;
}

static uint8_t plugin_ui_env_update_phase_do_loading(plugin_ui_env_t env, plugin_ui_phase_node_t top_node) {
    /*加载完成, 但是需要等待完成 */
    if (!top_node->m_loading_auto_complete) {
        /* 如果等待没有完成，则更小一下 */
        if (!TAILQ_EMPTY(&top_node->m_state_stack)) {
            plugin_ui_env_update_state(top_node, 0);
            if (!TAILQ_EMPTY(&top_node->m_state_stack)) {
                /*跟新后还是没有完成，则需直接返回*/
                return 0;
            }
        }
    }

    /*开始执行完成调整工作 */
    if (top_node->m_runtime_aspect) plugin_ui_aspect_clear(top_node->m_runtime_aspect);
    plugin_ui_phase_node_clear_stack(top_node);
    plugin_ui_phase_leave(top_node->m_loading_phase);

    /*清理掉loading阶段的资源 */
    plugin_package_group_clear(top_node->m_inout_packages);
    if (plugin_ui_env_sync_pages_and_packages(env, top_node, NULL, top_node->m_process_phase) != 0) {
        /*在进入加载阶段的时候处理过资源，此处不应该有错误，保护一下 */
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_env_update_phase: prepaire resource for enter process phase %s (from loading) fail!",
            plugin_ui_phase_name(top_node->m_process_phase));
        plugin_ui_phase_node_free(top_node);
        return 1;
    }

    if (plugin_ui_phase_enter(top_node->m_process_phase) != 0) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_env_update_phase: enter process phase %s (from loading) fail!",
            plugin_ui_phase_name(top_node->m_process_phase));
        plugin_ui_phase_node_free(top_node);
        return 1;
    }
    else {
        top_node->m_state = plugin_ui_phase_node_state_processing;
        top_node->m_is_processed = 0;
        if (env->m_debug) {
            CPE_INFO(env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s ==> %s", env->m_tick, plugin_ui_phase_node_name(top_node), plugin_ui_phase_node_state_str(top_node));
        }
        
        return 1;
    }
}

static uint8_t plugin_ui_env_update_phase_do_prepare_back(plugin_ui_env_t env, plugin_ui_phase_node_t top_node) {
    /*返回 */
    if (top_node->m_back_phase) {
        plugin_ui_phase_node_t pre_node = TAILQ_PREV(top_node, plugin_ui_phase_node_list, m_next);
        if (pre_node) {
            plugin_package_group_load_async(pre_node->m_runtime_packages, NULL);
        }
        
        /*如果有返回阶段，则需要执行返回阶段 */
        if (env->m_debug) {
            CPE_INFO(env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s do back begin", env->m_tick, plugin_ui_phase_node_name(top_node));
        }

        if (plugin_ui_phase_enter(top_node->m_back_phase) != 0) {
            CPE_ERROR(
                env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s: enter back phase %s fail!",
                env->m_tick, plugin_ui_phase_node_name(top_node), plugin_ui_phase_name(top_node->m_back_phase));
            plugin_ui_phase_node_free(top_node);
            return 1;
        }
        else {
            top_node->m_state = plugin_ui_phase_node_state_back;
            top_node->m_is_processed = 0;
            if (env->m_debug) {
                CPE_INFO(env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s ==> %s", env->m_tick, plugin_ui_phase_node_name(top_node), plugin_ui_phase_node_state_str(top_node));
            }
        }
    }
    else {
        /*没有返回阶段，则直接清理node */
        plugin_ui_phase_node_free(top_node);
    }
    
    return 1;
}

static uint8_t plugin_ui_env_update_phase_do_back(plugin_ui_env_t env, plugin_ui_phase_node_t top_node) {
    /*加载完成, 但是需要等待完成 */
    if (!top_node->m_back_auto_complete) {
        /* 如果等待没有完成，则更新一下 */
        if (!top_node->m_is_processed || !TAILQ_EMPTY(&top_node->m_state_stack)) {
            plugin_ui_env_update_state(top_node, 0);
            if (!TAILQ_EMPTY(&top_node->m_state_stack)) {
                /*跟新后还是没有完成，则需直接返回*/
                return 0;
            }
        }
    }

    if (env->m_debug) {
        CPE_INFO(env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s do back complete", env->m_tick, plugin_ui_phase_node_name(top_node));
    }

    plugin_ui_phase_node_free(top_node);
    return 1;
}

static uint8_t plugin_ui_env_update_phase_do_prepare_resume(plugin_ui_env_t env, plugin_ui_phase_node_t top_node) {
    /*top节点是suspend状态，则直接恢复节点到processin */
    if (plugin_ui_phase_enter(top_node->m_process_phase) != 0) {
        CPE_ERROR(
            env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s: suspend: enter process phase %s (from suspend) fail!",
            env->m_tick, plugin_ui_phase_node_name(top_node), plugin_ui_phase_name(top_node->m_process_phase));
        plugin_ui_phase_node_free(top_node);
        return 1;
    }
            
    top_node->m_state = plugin_ui_phase_node_state_processing;
    top_node->m_is_processed = 0;
    if (env->m_debug) {
        CPE_INFO(
            env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s ==> %s",
            env->m_tick, plugin_ui_phase_node_name(top_node), plugin_ui_phase_node_state_str(top_node));
    }

    do {
        plugin_ui_state_node_t state_node;
        TAILQ_FOREACH(state_node, &top_node->m_state_stack, m_next) {
            state_node->m_op = plugin_ui_state_node_op_resume;
        }
    } while(0);
    return 1;
}

int plugin_ui_env_set_init_phase(plugin_ui_env_t env, const char * init_phase_name, const char * init_call_phase_name) {
    plugin_ui_phase_t init_phase;
    plugin_ui_phase_t init_call_phase;
    uint8_t auto_complete = 0;

    init_phase = plugin_ui_phase_find(env, init_phase_name);
    if (init_phase == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_env_set_init_phase: init phase %s not exist!", init_phase_name);
        return -1;
    }

    init_call_phase = NULL;
    if (init_call_phase_name) {
        auto_complete = 1;
        
        if (init_call_phase_name[0] == '-') {
            auto_complete = 0;
            init_call_phase_name++;
        }
        
        init_call_phase = plugin_ui_phase_find(env, init_call_phase_name);
        if (init_call_phase == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_env_set_init_phase: init call phase %s not exist!", init_call_phase_name);
            return -1;
        }
    }

    env->m_init_phase = init_phase;
    env->m_init_call_phase = init_call_phase;
    env->m_init_call_phase_auto_complete = auto_complete;
    
    return 0;
}

plugin_ui_phase_t plugin_ui_env_init_phase(plugin_ui_env_t env) {
    return env->m_init_phase;
}

plugin_ui_phase_t plugin_ui_env_init_call_phase(plugin_ui_env_t env) {
    return env->m_init_call_phase;
}

int plugin_ui_env_phase_call(
    plugin_ui_env_t env, plugin_ui_phase_t phase,
    plugin_ui_phase_t loading_phase, uint8_t loading_auto_complete,
    plugin_ui_phase_t  back_phase, uint8_t back_auto_complete,
    dr_data_t data)
{
    plugin_ui_phase_node_t cur_phase_node;
    plugin_ui_phase_node_t pre_phase_node;

    if (env->m_debug) {
        CPE_INFO(
            env->m_module->m_em, "plugin_ui_env_do_phase_call: call to %s [loading=%s%s, back=%s%s]: begin",
            plugin_ui_phase_name(phase),
            loading_auto_complete ? "" : "-", loading_phase ? plugin_ui_phase_name(loading_phase) : "",
            back_auto_complete ? "" : "-", back_phase ? plugin_ui_phase_name(back_phase) : "");
    }

    pre_phase_node = TAILQ_LAST(&env->m_phase_stack, plugin_ui_phase_node_list);

    cur_phase_node = plugin_ui_phase_node_create(env, phase, loading_phase, loading_auto_complete, back_phase, back_auto_complete, data);
    if (cur_phase_node == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_env_do_phase_call: call to %s [loading=%s, back=%s], create phase nodes fail",
            plugin_ui_phase_name(phase),
            loading_phase ? plugin_ui_phase_name(loading_phase) : "",
            back_phase ? plugin_ui_phase_name(back_phase) : "");
        return -1;
    }

    cur_phase_node->m_op = plugin_ui_phase_node_op_init;
    if (pre_phase_node) {
        pre_phase_node->m_op = plugin_ui_phase_node_op_suspend;
    }

    return 0;
}

int plugin_ui_env_phase_call_by_name(
    plugin_ui_env_t env, const char * phase_name, const char * loading_phase_name, const char * back_phase_name, dr_data_t data)
{
    plugin_ui_phase_t phase, loading_phase, back_phase;
    uint8_t loading_auto_complete = 0;
    uint8_t back_auto_complete = 0;

    phase = plugin_ui_phase_find(env, phase_name);
    if (phase == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_env_do_phase_call: call to %s [loading=%s, back=%s]: phase not exist",
            phase_name, loading_phase_name ? loading_phase_name : "", back_phase_name ? back_phase_name : "");
        return -1;
    }

    loading_phase = NULL;
    if (loading_phase_name) {
        loading_auto_complete = 1;

        if (loading_phase_name[0] == '-') {
            loading_phase_name++;
            loading_auto_complete = 0;
        }
        
        loading_phase = plugin_ui_phase_find(env, loading_phase_name);
        if (loading_phase == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_env_do_phase_call: call to %s [loading=%s, back=%s]: loading phase not exist",
                phase_name, loading_phase_name ? loading_phase_name : "", back_phase_name ? back_phase_name : "");
            return -1;
        }
    }

    back_phase = NULL;
    if (back_phase_name) {
        back_auto_complete = 1;

        if (back_phase_name[0] == '-') {
            back_phase_name++;
            back_auto_complete = 0;
        }
        
        back_phase = plugin_ui_phase_find(env, back_phase_name);
        if (back_phase == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_env_do_phase_call: call to %s [loading=%s, back=%s]: back phase not exist",
                phase_name, loading_phase_name ? loading_phase_name : "", back_phase_name ? back_phase_name : "");
            return -1;
        }
    }
    
    return plugin_ui_env_phase_call(env, phase, loading_phase, loading_auto_complete, back_phase, back_auto_complete, data);
}

int plugin_ui_env_phase_reset(plugin_ui_env_t env) {
    plugin_ui_phase_node_t phase_node;

    if (env->m_debug) {
        CPE_INFO(env->m_module->m_em, "plugin_ui_env_phase_reset: begin");
    }
    
    TAILQ_FOREACH(phase_node, &env->m_phase_stack, m_next) {
        phase_node->m_op = plugin_ui_phase_node_op_remove;
    }

    return 0;
}

int plugin_ui_env_phase_switch(
    plugin_ui_env_t env, plugin_ui_phase_t phase,
    plugin_ui_phase_t loading_phase, uint8_t loading_auto_complete, dr_data_t data)
{
    plugin_ui_phase_node_t cur_phase_node;
    plugin_ui_phase_node_t pre_phase_node;

    pre_phase_node = TAILQ_LAST(&env->m_phase_stack, plugin_ui_phase_node_list);

    if (env->m_debug) {
        CPE_INFO(
            env->m_module->m_em, "plugin_ui_env_phase_switch: switch from %s to %s [loading=%s]: begin",
            pre_phase_node ? plugin_ui_phase_node_name(pre_phase_node) : "???",
            plugin_ui_phase_name(phase), loading_phase ? plugin_ui_phase_name(loading_phase) : "");
    }
    
    cur_phase_node = plugin_ui_phase_node_create(
        env, phase,
        loading_phase, loading_auto_complete,
        pre_phase_node ? pre_phase_node->m_back_phase : NULL,
        pre_phase_node ? pre_phase_node->m_back_auto_complete : 0,
        data);
    if (cur_phase_node == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_env_phase_switch: call to %s [loading=%s], create phase nodes fail",
            plugin_ui_phase_name(phase),
            loading_phase ? plugin_ui_phase_name(loading_phase) : "");
        return -1;
    }

    cur_phase_node->m_op = plugin_ui_phase_node_op_init;
    if (pre_phase_node) {
        pre_phase_node->m_op = plugin_ui_phase_node_op_remove;
    }
    
    return 0;
}

int plugin_ui_env_phase_switch_by_name(
    plugin_ui_env_t env, const char * phase_name, const char * loading_phase_name, dr_data_t data)
{
    plugin_ui_phase_t phase, loading_phase;
    uint8_t loading_auto_complete = 0;
    
    phase = plugin_ui_phase_find(env, phase_name);
    if (phase == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_env_phase_switch: switch to %s [loading=%s]: phase not exist",
            phase_name, loading_phase_name ? loading_phase_name : "");
        return -1;
    }

    loading_phase = NULL;
    if (loading_phase_name) {
        loading_auto_complete = 1;

        if (loading_phase_name[0] == '-') {
            loading_phase_name++;
            loading_auto_complete = 0;
        }
        
        loading_phase = plugin_ui_phase_find(env, loading_phase_name);
        if (loading_phase == NULL) {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_env_phase_switch: switch to %s [loading=%s]: loading phase not exist",
                phase_name, loading_phase_name ? loading_phase_name : "");
            return -1;
        }
    }

    return plugin_ui_env_phase_switch(env, phase, loading_phase, loading_auto_complete, data);
}

int plugin_ui_env_phase_back(plugin_ui_env_t env) {
    plugin_ui_phase_node_t phase_node;

    if (env->m_debug) {
        CPE_INFO(env->m_module->m_em, "plugin_ui_env_phase_back: begin");
    }
    
    phase_node = TAILQ_LAST(&env->m_phase_stack, plugin_ui_phase_node_list);
    if (phase_node == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_env_phase_back: no current node!");
        return -1;
    }
    
    phase_node->m_op = plugin_ui_phase_node_op_back;

    return 0;
}

static void plugin_ui_env_process_phase_ops(plugin_ui_env_t env) {
    plugin_ui_phase_node_t node, next;
    uint8_t init_op_count = 0;
    
    for(node = TAILQ_LAST(&env->m_phase_stack, plugin_ui_phase_node_list); node; node = next) {
        next = TAILQ_PREV(node, plugin_ui_phase_node_list, m_next);
        
        if (node->m_op == plugin_ui_phase_node_op_none) {
            break;
        }
        else if(node->m_op == plugin_ui_phase_node_op_init) {
            init_op_count++;
            continue;
        }
        else if(node->m_op == plugin_ui_phase_node_op_suspend) {
            /* 挂起 */
            if (node->m_state == plugin_ui_phase_node_state_back) {
                /* 在退出阶段，则直接退出，不再能够恢复 */
                plugin_ui_phase_node_free(node);
                continue;
            }
            else {
                /* 在loading或者process阶段，则挂起 */
                if (env->m_debug) {
                    CPE_INFO(env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s do suspend begin", env->m_tick, plugin_ui_phase_node_name(node));
                }
                
                if (node->m_runtime_aspect) plugin_ui_aspect_clear(node->m_runtime_aspect);

                if (node->m_state == plugin_ui_phase_node_state_loading) {
                    plugin_ui_phase_leave(node->m_loading_phase);
                }
                else if (node->m_state == plugin_ui_phase_node_state_processing) {
                    plugin_ui_phase_leave(node->m_process_phase);
                }

                plugin_package_group_set_using_state(node->m_inout_packages, plugin_package_package_using_state_free);
                plugin_package_group_set_using_state(node->m_runtime_packages, plugin_package_package_using_state_free);
                node->m_state = plugin_ui_phase_node_state_suspend;
                node->m_is_processed = 0; 
                if (env->m_debug) {
                    CPE_INFO(env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s ==> %s", env->m_tick, plugin_ui_phase_node_name(node), plugin_ui_phase_node_state_str(node));
                }
                
                plugin_ui_phase_node_suspend_stack(node);
            }
        }
        else if(node->m_op == plugin_ui_phase_node_op_resume) {
            /* 恢复资源状态 */
            plugin_package_group_set_using_state(node->m_inout_packages, plugin_package_package_using_state_ref_count);
            plugin_package_group_set_using_state(node->m_runtime_packages, plugin_package_package_using_state_ref_count);
            
            if (plugin_ui_env_sync_pages_and_packages(env, node, NULL, node->m_process_phase) != 0) {
                CPE_ERROR(env->m_module->m_em, "plugin_ui_env_update_phase: suspend: prepaire resources fail!");
                plugin_ui_phase_node_free(node);
                continue;
            }

            /* 加载所有运行时资源 */
            plugin_package_group_load_async(node->m_runtime_packages, plugin_ui_phase_node_create_package_load_task(node));
            node->m_state = plugin_ui_phase_node_state_prepare_resume;
            if (env->m_debug) {
                CPE_INFO(env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s ==> %s", env->m_tick, plugin_ui_phase_node_name(node), plugin_ui_phase_node_state_str(node));
            }
        }
        else if (node->m_op == plugin_ui_phase_node_op_remove) {
            /* 清除 */
            if (env->m_debug) {
                CPE_INFO(env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s do remove", env->m_tick, plugin_ui_phase_node_name(node));
            }
            
            plugin_ui_phase_node_free(node);
            continue;
        }
        else if (node->m_op == plugin_ui_phase_node_op_back) {
            plugin_ui_phase_node_t pre_node;
            plugin_package_load_task_t task;

            pre_node = TAILQ_PREV(node, plugin_ui_phase_node_list, m_next);
            if (pre_node == NULL) {
                plugin_ui_phase_node_free(node);
                continue;
            }
            
            /* 恢复资源状态 */
            plugin_package_group_set_using_state(pre_node->m_inout_packages, plugin_package_package_using_state_ref_count);
            plugin_package_group_set_using_state(pre_node->m_runtime_packages, plugin_package_package_using_state_ref_count);

            if (node->m_state == plugin_ui_phase_node_state_processing) {
                plugin_ui_phase_leave(node->m_process_phase);
            }
            else if (node->m_state == plugin_ui_phase_node_state_loading) {
                plugin_ui_phase_leave(node->m_loading_phase);
            }

            plugin_package_group_clear(node->m_runtime_packages);
            plugin_package_group_clear(node->m_inout_packages);
            if (node->m_back_phase) plugin_package_group_add_package_r(pre_node->m_inout_packages, node->m_back_phase->m_package);
                    
            plugin_package_module_total_reset(env->m_module->m_package_module);
            plugin_ui_env_sync_pages_and_packages(env, pre_node, node->m_back_phase, pre_node->m_process_phase);

            if (node->m_runtime_aspect) plugin_ui_aspect_clear(node->m_runtime_aspect);
            plugin_ui_phase_node_clear_stack(node);

            /* 返回时把需要的数据加载在前一个阶段的inout上 */
            task = plugin_ui_phase_node_create_package_load_task(node);
            plugin_package_group_load_async(pre_node->m_inout_packages, task);
            if (node->m_back_phase == NULL) {
                plugin_package_group_load_async(pre_node->m_runtime_packages, task);
            }
            
            node->m_state = plugin_ui_phase_node_state_prepare_back;
            if (env->m_debug) {
                CPE_INFO(env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s ==> %s", env->m_tick, plugin_ui_phase_node_name(node), plugin_ui_phase_node_state_str(node));
            }
        }
        else {
            assert(0);
            CPE_ERROR(env->m_module->m_em, "plugin_ui_env_process_phase_ops: unknown op %d", node->m_op);
        }
        
        node->m_op = plugin_ui_phase_node_op_none;
    }

    if (init_op_count > 0) {
        for(node = TAILQ_LAST(&env->m_phase_stack, plugin_ui_phase_node_list); node; node = next) {
            int rv = 0;
            plugin_package_load_task_t task;
            
            next = TAILQ_PREV(node, plugin_ui_phase_node_list, m_next);
            
            if(node->m_op != plugin_ui_phase_node_op_init) continue;

            if (env->m_debug) {
                CPE_INFO(env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s do init", env->m_tick, plugin_ui_phase_node_name(node));
            }

            /* 设置相关资源为锁定状态 */
            plugin_package_group_set_using_state(node->m_inout_packages, plugin_package_package_using_state_ref_count);
            plugin_package_group_set_using_state(node->m_runtime_packages, plugin_package_package_using_state_ref_count);

            /* 收集运行时资源 */
            if (node->m_loading_phase) {
                if (plugin_package_group_add_package_r(node->m_inout_packages, node->m_loading_phase->m_package) != 0) rv = -1;
            }
            if (plugin_package_group_add_package_r(node->m_runtime_packages, node->m_process_phase->m_package) != 0) rv = -1;

            /* 整理要需要的资源列表 */
            plugin_package_module_total_reset(env->m_module->m_package_module);
            if (plugin_ui_env_sync_pages_and_packages(env, node, node->m_loading_phase, node->m_process_phase) != 0) rv = -1;

            /* 如果没有加载阶段，所有资源必须在任务中加载完成，否则只需在加载阶段等待加载完成即可 */
            task = plugin_ui_phase_node_create_package_load_task(node);
            plugin_package_group_load_async(node->m_inout_packages, task);
            if (node->m_loading_phase == NULL) {
                plugin_package_group_load_async(node->m_runtime_packages, task);

                /*加载一次锁定包 */
                if (env->m_lock_packages == NULL) {
                    plugin_ui_env_init_lock_packages(env);
                    if (env->m_lock_packages) {
                        plugin_package_group_load_async(env->m_lock_packages, task);
                    }
                }
            }

            if (rv != 0) {
                CPE_ERROR(env->m_module->m_em, "plugin_ui_env_update_phase: prepaire resources fail!");
                plugin_ui_phase_node_free(node);
                continue;
            }
            
            node->m_op = plugin_ui_phase_node_op_none;

            node->m_state = plugin_ui_phase_node_state_prepare_loading;
            if (env->m_debug) {
                CPE_INFO(env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s ==> %s", env->m_tick, plugin_ui_phase_node_name(node), plugin_ui_phase_node_state_str(node));
            }
        }
    }
}

static int plugin_ui_env_sync_pages_and_packages(
    plugin_ui_env_t env, plugin_ui_phase_node_t phase_node, plugin_ui_phase_t inout, plugin_ui_phase_t runing)
{
    int rv = 0;
    plugin_ui_package_queue_managed_t managed_queue;
    plugin_ui_page_t page;
    plugin_ui_popup_t popup, pre_processed;
    
    /*先卸载不用的page */
    TAILQ_FOREACH(page, &env->m_pages, m_next_for_env) {
        if (inout && plugin_ui_phase_is_use_page(inout, page)) continue;
        if (runing && plugin_ui_phase_is_use_page(runing, page)) continue;

        if (plugin_ui_page_is_loaded(page)) {
            plugin_ui_page_unload(page);

            if (env->m_debug) {
                CPE_INFO(
                    env->m_module->m_em, "plugin_ui_env_sync_pages_and_packages: unload page %s success!",
                    plugin_ui_page_name(page));
            }
        }
    }

    /* 卸载不用的popup */

    pre_processed = NULL;
    for(popup = TAILQ_FIRST(&env->m_popups);
        popup;
        popup = pre_processed ? TAILQ_NEXT(pre_processed, m_next_for_env) : TAILQ_FIRST(&env->m_popups))
    {
        if (!plugin_ui_popup_visible(popup)) {
            plugin_ui_popup_free(popup);
            continue;
        }

        pre_processed = popup;
    }

    /* 修改队列资源的大小 */

    TAILQ_FOREACH(managed_queue, &env->m_package_queue_manageds, m_next_for_env) {
        plugin_ui_package_queue_using_t using;
        uint32_t limint = 0;
        
        if (inout && (using = plugin_ui_package_queue_using_find(managed_queue, inout))) {
            if (using->m_limit > limint) limint = using->m_limit;
        }

        if (runing && (using = plugin_ui_package_queue_using_find(managed_queue, runing))) {
            if (using->m_limit > limint) limint = using->m_limit;
        }

        plugin_package_queue_set_limit(managed_queue->m_package_queue, limint);
    }

    plugin_ui_env_package_gc(env);
    
    return rv;
}
