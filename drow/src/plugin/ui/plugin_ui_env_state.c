#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_stdio.h"
#include "plugin/package/plugin_package_module.h"
#include "plugin/package/plugin_package_group.h"
#include "plugin/package/plugin_package_load_task.h"
#include "plugin_ui_env_i.h"
#include "plugin_ui_state_i.h"
#include "plugin_ui_state_node_i.h"

typedef enum plugin_ui_env_update_state_next_op {
    plugin_ui_env_update_state_next_op_retry,
    plugin_ui_env_update_state_next_op_next,
    plugin_ui_env_update_state_next_op_retry_all,
    plugin_ui_env_update_state_next_op_break,
} plugin_ui_env_update_state_next_op_t;

static void plugin_ui_env_process_state_ops(plugin_ui_env_t env, plugin_ui_phase_node_t phase_node);

static plugin_ui_env_update_state_next_op_t plugin_ui_env_update_state_do_prepare_loading(plugin_ui_env_t env, plugin_ui_phase_node_t phase_node, plugin_ui_state_node_t process_node);
static plugin_ui_env_update_state_next_op_t plugin_ui_env_update_state_do_loading(plugin_ui_env_t env, plugin_ui_phase_node_t phase_node, plugin_ui_state_node_t process_node);
static plugin_ui_env_update_state_next_op_t plugin_ui_env_update_state_do_prepare_back(plugin_ui_env_t env, plugin_ui_phase_node_t phase_node, plugin_ui_state_node_t process_node);
static plugin_ui_env_update_state_next_op_t plugin_ui_env_update_state_do_back(plugin_ui_env_t env, plugin_ui_phase_node_t phase_node, plugin_ui_state_node_t process_node);
static plugin_ui_env_update_state_next_op_t plugin_ui_env_update_state_do_prepare_resume(plugin_ui_env_t env, plugin_ui_phase_node_t phase_node, plugin_ui_state_node_t process_node);
static plugin_ui_env_update_state_next_op_t plugin_ui_env_update_state_do_process(plugin_ui_env_t env, plugin_ui_phase_node_t phase_node, plugin_ui_state_node_t process_node);

#define plugin_ui_env_update_state_next_op_do(__op)     \
    switch((__op)) {                                    \
    case plugin_ui_env_update_state_next_op_retry:      \
        next_process_node = process_node;               \
        break;                                          \
    case plugin_ui_env_update_state_next_op_retry_all:  \
        goto UPDATE_TRY_AGAIN;                          \
    case plugin_ui_env_update_state_next_op_break:      \
        next_process_node = NULL;                       \
        break;                                          \
    case plugin_ui_env_update_state_next_op_next:       \
        break;                                          \
    default:                                            \
        break;                                          \
    }                                                   \

void plugin_ui_env_update_state(plugin_ui_phase_node_t phase_node, uint8_t can_restart) {
    plugin_ui_env_t env = phase_node->m_env;
    plugin_ui_state_node_t process_node, next_process_node;
    uint8_t is_init = 0;

    /* CPE_ERROR( */
    /*     env->m_module->m_em, "xxxxx: update begin: %s", */
    /*     plugin_ui_phase_node_dump(&env->m_module->m_dump_buffer, phase_node)); */
    
    phase_node->m_is_processed = 1;
    
UPDATE_TRY_AGAIN:
    if (TAILQ_EMPTY(&phase_node->m_state_stack)) {
        plugin_ui_phase_t cur_phase;
        plugin_ui_state_node_t state_node;
        
        cur_phase = plugin_ui_phase_node_current_phase(phase_node);
        assert(cur_phase);

        if (cur_phase->m_init_state == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_env_update_state: phase %s no init state!", plugin_ui_phase_name(cur_phase));
            return;
        }

        state_node = plugin_ui_state_node_create(
                phase_node, cur_phase->m_init_state, cur_phase->m_init_call_state, NULL, 0,
                phase_node->m_data.m_data ? &phase_node->m_data : NULL);
        if (state_node == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_env_update_state: create first state node fail!");
            return;
        }
        is_init = 1;
        state_node->m_op = plugin_ui_state_node_op_init;
    }

    plugin_ui_env_process_state_ops(env, phase_node);
    
    /*从堆栈底部向上遍历所有需要处理的node */
    for(process_node = TAILQ_FIRST(&phase_node->m_state_stack); process_node; process_node = next_process_node) {
        next_process_node = TAILQ_NEXT(process_node, m_next);

        if (process_node->m_op == plugin_ui_state_node_op_init) continue;
        
        switch(process_node->m_state) {
        case plugin_ui_state_node_state_prepare_loading:
            if (plugin_ui_state_node_check_package_load_task_runing(process_node)) {
                CPE_INFO(
                    env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s: state %s: init: still waiting",
                    env->m_tick, plugin_ui_phase_node_name(phase_node), plugin_ui_state_node_name(process_node));
                continue;
            }

            plugin_ui_env_update_state_next_op_do(
                plugin_ui_env_update_state_do_prepare_loading(env, phase_node, process_node));
            continue;
        case plugin_ui_state_node_state_loading:
            /*加载未完成 */
            if ((plugin_package_module_total_load_complete_count(env->m_module->m_package_module)
                 != plugin_package_module_total_load_count(env->m_module->m_package_module))
                || plugin_ui_state_node_is_active(process_node))
            {
                continue;
            }

            plugin_ui_env_update_state_next_op_do(
                plugin_ui_env_update_state_do_loading(env, phase_node, process_node));
            break;
        case plugin_ui_state_node_state_prepare_back:
            if (plugin_ui_state_node_check_package_load_task_runing(process_node)) {
                CPE_INFO(
                    env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s: state %s: back: still waiting",
                    env->m_tick, plugin_ui_phase_node_name(phase_node), plugin_ui_state_node_name(process_node));
                continue;
            }

            plugin_ui_env_update_state_next_op_do(
                plugin_ui_env_update_state_do_prepare_back(env, phase_node, process_node));
            break;
        case plugin_ui_state_node_state_back:
            /*加载未完成 */
            if ((plugin_package_module_total_load_complete_count(env->m_module->m_package_module)
                 != plugin_package_module_total_load_count(env->m_module->m_package_module))
                || plugin_ui_state_node_is_active(process_node))
            {
                continue;
            }

            plugin_ui_env_update_state_next_op_do(
                plugin_ui_env_update_state_do_back(env, phase_node, process_node));
            break;
        case plugin_ui_state_node_state_suspend:
            continue;
        case plugin_ui_state_node_state_prepare_resume:
            if (plugin_ui_state_node_check_package_load_task_runing(process_node)) {
                CPE_INFO(
                    env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s: state %s: resume: still waiting",
                    env->m_tick, plugin_ui_phase_node_name(phase_node), plugin_ui_state_node_name(process_node));
                continue;
            }

            plugin_ui_env_update_state_next_op_do(
                plugin_ui_env_update_state_do_prepare_resume(env, phase_node, process_node));
            break;
        case plugin_ui_state_node_state_processing:
            if (plugin_ui_state_node_is_active(process_node)) continue;

            plugin_ui_env_update_state_next_op_do(
                plugin_ui_env_update_state_do_process(env, phase_node, process_node));
            break;
        default:
            CPE_ERROR(env->m_module->m_em, "plugin_ui_env_update_state: unknown state node state!");
            plugin_ui_state_node_free(process_node);
            break;
        }
    }

    if (TAILQ_EMPTY(&phase_node->m_state_stack)) {
        if (can_restart && phase_node->m_state == plugin_ui_phase_node_state_processing && !is_init) goto UPDATE_TRY_AGAIN;
    }

    /* CPE_ERROR( */
    /*     env->m_module->m_em, "xxxxx: update complete: %s", */
    /*     plugin_ui_phase_node_dump(&env->m_module->m_dump_buffer, phase_node)); */
}

plugin_ui_state_node_t
plugin_ui_env_state_call(
    plugin_ui_env_t env, const char * state_name, const char * loading_state_name, const char * back_state_name,
    plugin_ui_renter_policy_t renter_policy, uint8_t suspend_old, dr_data_t data)
{
    plugin_ui_phase_node_t phase_node;
    plugin_ui_state_node_t state_node;

    phase_node = plugin_ui_phase_node_current(env);
    if (phase_node == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_env_state_call: no current phase!");
        return NULL;
    }

    state_node = plugin_ui_state_node_current(phase_node);
    if (state_node == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_env_state_call: no current state!");
        return NULL;
    }

    return plugin_ui_state_node_call(state_node, state_name, loading_state_name, back_state_name, renter_policy, suspend_old, data);
}

int plugin_ui_env_state_switch(
    plugin_ui_env_t env, const char * state_name, const char * loading_state_name, const char * back_state_name, dr_data_t data)
{
    plugin_ui_phase_node_t phase_node;
    plugin_ui_state_node_t state_node;

    phase_node = plugin_ui_phase_node_current(env);
    if (phase_node == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_env_state_switch: no current phase!");
        return -1;
    }

    state_node = plugin_ui_state_node_current(phase_node);
    if (state_node == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_env_state_switch: no current state!");
        return -1;
    }

    return plugin_ui_state_node_switch(state_node, state_name, loading_state_name, back_state_name, data);
}

int plugin_ui_env_state_reset(plugin_ui_env_t env) {
    plugin_ui_phase_node_t phase_node;

    phase_node = plugin_ui_phase_node_current(env);
    if (phase_node == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_env_state_reset: no current phase!");
        return -1;
    }

    return plugin_ui_state_node_reset(phase_node);
}

int plugin_ui_env_state_back(plugin_ui_env_t env) {
    plugin_ui_phase_node_t phase_node;
    plugin_ui_state_node_t state_node;

    phase_node = plugin_ui_phase_node_current(env);
    if (phase_node == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_env_state_back: no current phase!");
        return -1;
    }

    state_node = plugin_ui_state_node_current(phase_node);
    if (state_node == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_env_state_back: no current state!");
        return -1;
    }

    plugin_ui_state_node_back(state_node);
    
    return 0;
}

static void plugin_ui_env_process_state_ops(plugin_ui_env_t env, plugin_ui_phase_node_t phase_node) {
    uint8_t init_op_count = 0;
    uint8_t inout_op_count = 0;
    plugin_ui_state_node_t process_node, next_process_node;
    
    /*首先处理堆栈顶部的请求，如果是等待进入，则跳过，如果回退或者删除操作，则处理 */
    for(process_node = TAILQ_LAST(&phase_node->m_state_stack, plugin_ui_state_node_list);
        process_node;
        process_node = next_process_node)
    {
        next_process_node = TAILQ_PREV(process_node, plugin_ui_state_node_list, m_next);

        if(process_node->m_op == plugin_ui_state_node_op_none) {
            switch(process_node->m_state) {
            case plugin_ui_state_node_state_prepare_loading:
            case plugin_ui_state_node_state_loading:
            case plugin_ui_state_node_state_prepare_back:
            case plugin_ui_state_node_state_back:
            case plugin_ui_state_node_state_prepare_resume:
                inout_op_count++;
                break;
            default:
                break;
            }

            if (inout_op_count > 0) break;
        }
        else if (process_node->m_op == plugin_ui_state_node_op_init) {
            init_op_count++;
            /*init是回退以后需要重新进入的节点，直接略过 */
            continue;
        }
        else if(process_node->m_op == plugin_ui_state_node_op_remove) {
            plugin_ui_state_node_free(process_node);
            continue;
        }
        else if (process_node->m_op == plugin_ui_state_node_op_back) {
            plugin_ui_state_node_suspend(process_node);

            if (process_node->m_curent.m_back_state && process_node->m_curent.m_back_state->m_packages) {
                if (plugin_package_group_add_packages_r(
                        plugin_ui_state_node_inout_packages_check_create(process_node),
                        process_node->m_curent.m_back_state->m_packages) != 0)
                {
                    CPE_ERROR(env->m_module->m_em, "plugin_ui_env_process_state_ops: add inout pacakges fail!");
                }
            }
            
            process_node->m_op = plugin_ui_state_node_op_none;
            process_node->m_state = plugin_ui_state_node_state_prepare_back;
            if (env->m_debug) {
                CPE_INFO(
                    env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s: state %s ==> %s",
                    env->m_tick, plugin_ui_phase_node_name(phase_node), plugin_ui_state_node_name(process_node), plugin_ui_state_node_state_str(process_node));
            }

            inout_op_count++;
            break;
        }
        else if (process_node->m_op == plugin_ui_state_node_op_switch) {
            plugin_ui_state_node_suspend(process_node);

            if (process_node->m_curent.m_inout_packages) {
                plugin_package_group_free(process_node->m_curent.m_inout_packages);
                process_node->m_curent.m_inout_packages = NULL;
                env->m_package_need_gc = 1;
            }

            if (process_node->m_curent.m_runtime_packages) {
                plugin_package_group_free(process_node->m_curent.m_runtime_packages);
                process_node->m_curent.m_runtime_packages = NULL;
                env->m_package_need_gc = 1;
            }

            if (process_node->m_replace.m_process_state) {
                plugin_ui_state_node_replace(process_node, 1);
                process_node->m_state = plugin_ui_state_node_state_prepare_loading;
                process_node->m_op = plugin_ui_state_node_op_init;
                init_op_count++;
            }   
            else {
                plugin_ui_state_node_free(process_node);
            }
            
            continue;
        }
        else if (process_node->m_op == plugin_ui_state_node_op_resume) {
            if (process_node->m_curent.m_inout_packages) {
                plugin_package_group_set_using_state(process_node->m_curent.m_inout_packages, plugin_package_package_using_state_ref_count);
            }

            if (process_node->m_curent.m_runtime_packages) {
                plugin_package_group_set_using_state(process_node->m_curent.m_runtime_packages, plugin_package_package_using_state_ref_count);
            }

            if (process_node->m_curent.m_runtime_packages) {
                plugin_package_load_task_t task;

                if (env->m_package_need_gc) plugin_ui_env_package_gc(env);
                
                /* 如果没有加载阶段，所有资源必须在任务中加载完成，否则只需在加载阶段等待加载完成即可 */
                task = plugin_ui_state_node_create_package_load_task(process_node);
                plugin_package_group_load_async(process_node->m_curent.m_runtime_packages, task);
            }


            process_node->m_op = plugin_ui_state_node_op_none;
            process_node->m_state = plugin_ui_state_node_state_prepare_resume;
            inout_op_count++;
            break;
        }
    }

    if (init_op_count > 0 && inout_op_count == 0) {
        for(process_node = TAILQ_FIRST(&phase_node->m_state_stack); process_node; process_node = next_process_node) {
            next_process_node = TAILQ_NEXT(process_node, m_next);

            if (process_node->m_op != plugin_ui_state_node_op_init) continue;

            /* 收集运行时资源 */
            if (process_node->m_curent.m_loading_state && process_node->m_curent.m_loading_state->m_packages) {
                if (plugin_package_group_add_packages_r(
                        plugin_ui_state_node_inout_packages_check_create(process_node),
                        process_node->m_curent.m_loading_state->m_packages) != 0)
                {
                    CPE_ERROR(env->m_module->m_em, "plugin_ui_env_process_state_ops: add inout pacakges fail!");
                }
            }

            if (process_node->m_curent.m_process_state && process_node->m_curent.m_process_state->m_packages) {
                if (plugin_package_group_add_packages_r(
                        plugin_ui_state_node_runtime_packages_check_create(process_node),
                        process_node->m_curent.m_process_state->m_packages) != 0) {
                    CPE_ERROR(env->m_module->m_em, "plugin_ui_env_process_state_ops: add runing pacakges fail!");
                }
            }

            /* 设置相关资源为锁定状态 */
            if (process_node->m_curent.m_inout_packages) {
                plugin_package_group_set_using_state(process_node->m_curent.m_inout_packages, plugin_package_package_using_state_ref_count);
            }

            if (process_node->m_curent.m_runtime_packages) {
                plugin_package_group_set_using_state(process_node->m_curent.m_runtime_packages, plugin_package_package_using_state_ref_count);
            }

            plugin_package_module_total_reset(env->m_module->m_package_module);
            if (process_node->m_curent.m_inout_packages != NULL || process_node->m_curent.m_runtime_packages != NULL) {
                plugin_package_load_task_t task;

                if (env->m_package_need_gc) plugin_ui_env_package_gc(env);
                
                /* 如果没有加载阶段，所有资源必须在任务中加载完成，否则只需在加载阶段等待加载完成即可 */
                task = plugin_ui_state_node_create_package_load_task(process_node);
                if (process_node->m_curent.m_inout_packages) {
                    plugin_package_group_load_async(process_node->m_curent.m_inout_packages, task);
                }
                else {
                    plugin_package_group_load_async(process_node->m_curent.m_runtime_packages, task);
                }
            }
            
            process_node->m_state = plugin_ui_state_node_state_prepare_loading;
            process_node->m_op = plugin_ui_state_node_op_none;
            if (env->m_debug) {
                CPE_INFO(
                    env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s: state %s ==> %s (init)",
                    env->m_tick, plugin_ui_phase_node_name(phase_node), plugin_ui_state_node_name(process_node), plugin_ui_state_node_state_str(process_node));
            }
            break;
        }
    }
}

static plugin_ui_env_update_state_next_op_t
plugin_ui_env_update_state_do_prepare_loading(plugin_ui_env_t env, plugin_ui_phase_node_t phase_node, plugin_ui_state_node_t process_node) {
    if (process_node->m_curent.m_loading_state) {
        if (process_node->m_curent.m_runtime_packages) {
            if (env->m_package_need_gc) plugin_ui_env_package_gc(env);
            plugin_package_group_load_async(process_node->m_curent.m_runtime_packages, NULL);
        }

        if (plugin_ui_state_node_active(process_node, plugin_ui_state_node_state_loading) != 0) {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_env_update_state: enter loading state %s (from init) fail, skip!",
                plugin_ui_state_name(process_node->m_curent.m_loading_state));
            plugin_ui_state_node_free(process_node);
            return plugin_ui_env_update_state_next_op_next;
        }
        
        return plugin_ui_env_update_state_next_op_retry;
    }
    else {
        if (plugin_ui_state_node_active(process_node, plugin_ui_state_node_state_processing) != 0) {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_env_update_state: enter process state %s (from init) fail!",
                plugin_ui_state_name(process_node->m_curent.m_process_state));
            plugin_ui_state_node_free(process_node);
            return plugin_ui_env_update_state_next_op_next;
        }

        return plugin_ui_env_update_state_next_op_retry_all;
    }
}

static plugin_ui_env_update_state_next_op_t
plugin_ui_env_update_state_do_loading(plugin_ui_env_t env, plugin_ui_phase_node_t phase_node, plugin_ui_state_node_t process_node) {
    plugin_ui_state_node_suspend(process_node);
    
    if (plugin_ui_env_debug(env)) {
        CPE_INFO(
            env->m_module->m_em, "plugin_ui_env_update_state: level %d: loading state %s done!",
            process_node->m_level, plugin_ui_state_name(process_node->m_curent.m_loading_state));
    }

    if (process_node->m_curent.m_inout_packages) {
        plugin_package_group_free(process_node->m_curent.m_inout_packages);
        process_node->m_curent.m_inout_packages = NULL;
        env->m_package_need_gc = 1;
    }
    
    if (plugin_ui_state_node_active(process_node, plugin_ui_state_node_state_processing) != 0) {
        CPE_ERROR(
            env->m_module->m_em, "plugin_ui_env_update_state: enter process state %s (from loading) fail!",
            plugin_ui_state_name(process_node->m_curent.m_process_state));
        plugin_ui_state_node_free(process_node);
        return plugin_ui_env_update_state_next_op_next;
    }

    process_node->m_state = plugin_ui_state_node_state_processing;
    if (plugin_ui_env_debug(env)) {
        CPE_INFO(
            env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s: state %s ==> %s",
            env->m_tick, plugin_ui_phase_node_name(phase_node), plugin_ui_state_node_name(process_node), plugin_ui_state_node_state_str(process_node));
    }
    
    return plugin_ui_env_update_state_next_op_retry_all;
}

static plugin_ui_env_update_state_next_op_t
plugin_ui_env_update_state_do_prepare_back(plugin_ui_env_t env, plugin_ui_phase_node_t phase_node, plugin_ui_state_node_t process_node) {
    if (process_node->m_curent.m_back_state) {
        /*如果有返回阶段，则需要执行返回阶段 */
        if (plugin_ui_state_node_active(process_node, plugin_ui_state_node_state_back) != 0) {
            CPE_ERROR(
                env->m_module->m_em, "plugin_ui_env_update_state: level %d: process state %s back fail",
                process_node->m_level, plugin_ui_state_name(process_node->m_curent.m_process_state));
            plugin_ui_state_node_free(process_node);
            return plugin_ui_env_update_state_next_op_next;
        }
        else {
            /*正常进入返回阶段，则认为返回阶段已经执行，跳过后续检查 */
            return 0;
        }
    }
    else {
        if (process_node->m_curent.m_runtime_packages) {
            plugin_package_group_free(process_node->m_curent.m_runtime_packages);
            process_node->m_curent.m_runtime_packages = NULL;
            env->m_package_need_gc = 1;
        }
            
        /*有需要充入的操作，则开始执行重入操作 */
        if (process_node->m_replace.m_process_state) {
            plugin_ui_state_node_replace(process_node, 1);
            process_node->m_state = plugin_ui_state_node_state_prepare_loading;
            process_node->m_op = plugin_ui_state_node_op_init;
            
            if (plugin_ui_env_debug(env)) {
                CPE_INFO(
                    env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s: state %s ==> %s (prepare back replace)",
                    env->m_tick, plugin_ui_phase_node_name(phase_node), plugin_ui_state_node_name(process_node), plugin_ui_state_node_state_str(process_node));
            }
            return plugin_ui_env_update_state_next_op_retry_all;
        }   
        else {
            /*没有任何充入操作，清理并返回 */
            plugin_ui_state_node_free(process_node);
            return plugin_ui_env_update_state_next_op_next;
        }
    }
}

static plugin_ui_env_update_state_next_op_t
plugin_ui_env_update_state_do_back(plugin_ui_env_t env, plugin_ui_phase_node_t phase_node, plugin_ui_state_node_t process_node) {
    plugin_ui_state_node_suspend(process_node);

    if (process_node->m_curent.m_inout_packages) {
        plugin_package_group_free(process_node->m_curent.m_inout_packages);
        process_node->m_curent.m_inout_packages = NULL;
        env->m_package_need_gc = 1;
    }
    
    if (process_node->m_curent.m_runtime_packages) {
        plugin_package_group_free(process_node->m_curent.m_runtime_packages);
        process_node->m_curent.m_runtime_packages = NULL;
        env->m_package_need_gc = 1;
    }
    
    if (process_node->m_replace.m_process_state) {
        if (plugin_ui_env_debug(env)) {
            CPE_INFO(
                env->m_module->m_em, "plugin_ui_env_update_state: level %d: back state %s done, auto replace!",
                process_node->m_level, plugin_ui_state_name(process_node->m_curent.m_back_state));
        }

        plugin_ui_state_node_replace(process_node, 1);
        process_node->m_state = plugin_ui_state_node_state_prepare_loading;
        process_node->m_op = plugin_ui_state_node_op_init;
        if (plugin_ui_env_debug(env)) {
            CPE_INFO(
                env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s: state %s ==> %s (back replace)",
                env->m_tick, plugin_ui_phase_node_name(phase_node), plugin_ui_state_node_name(process_node), plugin_ui_state_node_state_str(process_node));
        }
        return plugin_ui_env_update_state_next_op_retry_all;
    }
    else {
        if (plugin_ui_env_debug(env)) {
            CPE_INFO(
                env->m_module->m_em, "plugin_ui_env_update_state: level %d: back state %s done!",
                process_node->m_level, plugin_ui_state_name(process_node->m_curent.m_back_state));
        }

        plugin_ui_state_node_free(process_node);
        return plugin_ui_env_update_state_next_op_next;
    }
}

static plugin_ui_env_update_state_next_op_t
plugin_ui_env_update_state_do_prepare_resume(plugin_ui_env_t env, plugin_ui_phase_node_t phase_node, plugin_ui_state_node_t process_node) {            
    if (plugin_ui_state_node_active(process_node, plugin_ui_state_node_state_processing) == 0) {
        plugin_ui_state_node_free(process_node);
        return plugin_ui_env_update_state_next_op_next;
    }
    assert(process_node->m_state == plugin_ui_state_node_state_processing);
    
    if (plugin_ui_env_debug(env)) {
        CPE_INFO(
            env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s: state %s ==> %s (prepare resume)",
            env->m_tick, plugin_ui_phase_node_name(phase_node), plugin_ui_state_node_name(process_node), plugin_ui_state_node_state_str(process_node));
    }
    
    return plugin_ui_env_update_state_next_op_retry_all;
}

static plugin_ui_env_update_state_next_op_t
plugin_ui_env_update_state_do_process(plugin_ui_env_t env, plugin_ui_phase_node_t phase_node, plugin_ui_state_node_t process_node) {
    plugin_ui_state_node_suspend(process_node);
    
    /*当前的Process完成了, 尝试保护 */
    if (plugin_ui_env_debug(env)) {
        CPE_INFO(
            env->m_module->m_em, "plugin_ui_env_update_state: level %d: process state %s done, auto back",
            process_node->m_level, plugin_ui_state_name(process_node->m_curent.m_process_state));
    }

    for(; process_node; process_node = TAILQ_NEXT(process_node, m_next)) {
        process_node->m_op = plugin_ui_state_node_op_back;
        plugin_ui_state_node_data_fini(env, &process_node->m_replace);
    }

    return plugin_ui_env_update_state_next_op_retry_all;
}
