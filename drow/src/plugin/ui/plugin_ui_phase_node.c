#include <assert.h>
#include "cpe/utils/stream_buffer.h"
#include "cpe/pal/pal_stdio.h"
#include "render/cache/ui_cache_group.h"
#include "render/cache/ui_cache_res.h"
#include "render/model/ui_data_src_group.h"
#include "render/runtime/ui_runtime_module.h"
#include "plugin/package/plugin_package_group.h"
#include "plugin/package/plugin_package_load_task.h"
#include "plugin_ui_phase_node_i.h"
#include "plugin_ui_state_node_i.h"
#include "plugin_ui_aspect_i.h"

plugin_ui_phase_node_t
plugin_ui_phase_node_create(
    plugin_ui_env_t env,
    plugin_ui_phase_t process_phase,
    plugin_ui_phase_t loading_phase, uint8_t loading_auto_complete,
    plugin_ui_phase_t back_phase, uint8_t back_auto_complete,
    dr_data_t data)
{
    plugin_ui_phase_node_t phase_node;
    char group_name[64];

    phase_node = TAILQ_FIRST(&env->m_free_phase_nodes);
    if (phase_node) {
        TAILQ_REMOVE(&env->m_free_phase_nodes, phase_node, m_next);
    }
    else {
        phase_node = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_phase_node));
        if (phase_node == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_phase_node_create: alloc fail!");
            return NULL;
        }
    }

    phase_node->m_env = env;
    phase_node->m_inout_packages = env->m_next_phase_loading_packages;
    phase_node->m_runtime_packages = env->m_next_phase_runing_packages;
    phase_node->m_package_load_task = 0;
    phase_node->m_state = plugin_ui_phase_node_state_prepare_loading;
    phase_node->m_op = plugin_ui_phase_node_op_none;
    phase_node->m_process_phase = process_phase;
    phase_node->m_loading_phase = loading_phase;
    phase_node->m_loading_auto_complete = loading_auto_complete;
    phase_node->m_back_phase = back_phase;
    phase_node->m_back_auto_complete = back_auto_complete;
    phase_node->m_runtime_aspect = NULL;

    env->m_next_phase_loading_packages = NULL;
    env->m_next_phase_runing_packages = NULL;
    
    if (data) {
        phase_node->m_data.m_meta = data->m_meta;
        phase_node->m_data.m_size = data->m_size;
        if (data->m_size <= CPE_ARRAY_SIZE(phase_node->m_data_inline_buf)) {
            phase_node->m_data.m_data = phase_node->m_data_inline_buf;
        }
        else {
            phase_node->m_data.m_data = mem_alloc(env->m_module->m_alloc, data->m_size);
            if (phase_node->m_data.m_data == NULL) {
                CPE_ERROR(env->m_module->m_em, "plugin_ui_phase_node_data_init: malloc data buf fail!");
                goto CREATE_FAIL;
            }
        }
        memcpy(phase_node->m_data.m_data, data->m_data, data->m_size);
    }
    else {
        phase_node->m_data.m_meta = NULL;
        phase_node->m_data.m_size = 0;
        phase_node->m_data.m_data = NULL;
    }

    snprintf(group_name, sizeof(group_name), "phase.%s.inout", plugin_ui_phase_node_name(phase_node));
    if (phase_node->m_inout_packages == NULL) {
        phase_node->m_inout_packages = plugin_package_group_create(env->m_module->m_package_module, group_name);
        if (phase_node->m_inout_packages == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_phase_node_create: create inout packages fail!");
            goto CREATE_FAIL;
        }
    }
    else {
        plugin_package_group_set_name(phase_node->m_inout_packages, group_name);
    }        
    
    snprintf(group_name, sizeof(group_name), "phase.%s", plugin_ui_phase_node_name(phase_node));
    if (phase_node->m_runtime_packages == NULL) {
        phase_node->m_runtime_packages = plugin_package_group_create(env->m_module->m_package_module, group_name);
        if (phase_node->m_runtime_packages == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_phase_node_create: create runtime packages fail!");
            goto CREATE_FAIL;
        }
    }
    else {
        plugin_package_group_set_name(phase_node->m_runtime_packages, group_name);
    }
        
    TAILQ_INIT(&phase_node->m_state_stack);
    
    TAILQ_INSERT_TAIL(&env->m_phase_stack, phase_node, m_next);

    return phase_node;

CREATE_FAIL:
    if (phase_node->m_inout_packages) {
        plugin_package_group_free(phase_node->m_inout_packages);
    }
    
    if (phase_node->m_runtime_packages) {
        plugin_package_group_free(phase_node->m_runtime_packages);
    }
    
    if (phase_node->m_data.m_data && phase_node->m_data.m_data != phase_node->m_data_inline_buf) {
        mem_free(env->m_module->m_alloc, phase_node->m_data.m_data);
    }
    
    TAILQ_INSERT_TAIL(&env->m_free_phase_nodes, phase_node, m_next);

    return NULL;
}

void plugin_ui_phase_node_free(plugin_ui_phase_node_t phase_node) {
    plugin_ui_env_t env = phase_node->m_env;

    plugin_ui_phase_node_stop_package_load_task(phase_node);
    
    if (phase_node->m_runtime_aspect) plugin_ui_aspect_clear(phase_node->m_runtime_aspect);

    plugin_ui_phase_node_clear_stack(phase_node);

    switch(phase_node->m_state) {
    case plugin_ui_phase_node_state_loading:
        plugin_ui_phase_leave(phase_node->m_loading_phase);
        break;
    case plugin_ui_phase_node_state_processing:
        plugin_ui_phase_leave(phase_node->m_process_phase);
        break;
    case plugin_ui_phase_node_state_back:
        plugin_ui_phase_leave(phase_node->m_back_phase);
        break;
    default:
        break;
    }

    if (phase_node->m_runtime_aspect) {
        plugin_ui_aspect_free(phase_node->m_runtime_aspect);
        phase_node->m_runtime_aspect = NULL;
    }

    if (phase_node->m_inout_packages) {
        plugin_package_group_free(phase_node->m_inout_packages);
        phase_node->m_inout_packages = NULL;
    }
    
    plugin_package_group_free(phase_node->m_runtime_packages);
    phase_node->m_runtime_packages = NULL;

    if (phase_node->m_data.m_data && phase_node->m_data.m_data != phase_node->m_data_inline_buf) {
        mem_free(env->m_module->m_alloc, phase_node->m_data.m_data);
    }
    
    TAILQ_REMOVE(&env->m_phase_stack, phase_node, m_next);

    TAILQ_INSERT_TAIL(&env->m_free_phase_nodes, phase_node, m_next);
}

void plugin_ui_phase_node_real_free(plugin_ui_phase_node_t phase_node) {
    plugin_ui_env_t env = phase_node->m_env;
    
    TAILQ_REMOVE(&env->m_free_phase_nodes, phase_node, m_next);

    mem_free(env->m_module->m_alloc, phase_node);
}

plugin_ui_aspect_t plugin_ui_phase_node_runtime_aspect(plugin_ui_phase_node_t phase_node) {
    if (phase_node->m_runtime_aspect == NULL) {
        phase_node->m_runtime_aspect = plugin_ui_aspect_create(phase_node->m_env, "*phase-runtime");
        assert(phase_node->m_runtime_aspect);
    }

    return phase_node->m_runtime_aspect;
}

plugin_ui_phase_node_t plugin_ui_phase_node_current(plugin_ui_env_t env) {
    return TAILQ_LAST(&env->m_phase_stack, plugin_ui_phase_node_list);
}

plugin_ui_phase_node_t plugin_ui_phase_node_prev(plugin_ui_phase_node_t phase_node) {
    return TAILQ_PREV(phase_node, plugin_ui_phase_node_list, m_next);
}

const char * plugin_ui_phase_node_name(plugin_ui_phase_node_t phase_node) {
    return phase_node->m_process_phase->m_name;
}

plugin_ui_phase_node_state_t plugin_ui_phase_node_state(plugin_ui_phase_node_t phase_node) {
    return phase_node->m_state;
}

const char * plugin_ui_phase_node_op_str(plugin_ui_phase_node_t phase_node) {
    return plugin_ui_phase_node_op_to_str(phase_node->m_op);
}

const char * plugin_ui_phase_node_state_str(plugin_ui_phase_node_t phase_node) {
    return plugin_ui_phase_node_state_to_str(phase_node->m_state);
}

plugin_ui_phase_t plugin_ui_phase_node_loading_phase(plugin_ui_phase_node_t phase_node) {
    return phase_node->m_loading_phase;
}

plugin_ui_phase_t plugin_ui_phase_node_process_phase(plugin_ui_phase_node_t phase_node) {
    return phase_node->m_process_phase;
}

plugin_ui_phase_t plugin_ui_phase_node_back_phase(plugin_ui_phase_node_t phase_node) {
    return phase_node->m_back_phase;
}

plugin_ui_phase_t plugin_ui_phase_node_current_phase(plugin_ui_phase_node_t phase_node) {
    switch(phase_node->m_state) {
    case plugin_ui_phase_node_state_loading:
        return phase_node->m_loading_phase;
    case plugin_ui_phase_node_state_processing:
        return phase_node->m_process_phase;
    case plugin_ui_phase_node_state_back:
        return phase_node->m_back_phase;
    default:
        return NULL;
    }
}

const char * plugin_ui_phase_node_current_phase_name(plugin_ui_phase_node_t phase_node) {
    plugin_ui_phase_t phase = plugin_ui_phase_node_current_phase(phase_node);
    return phase ? plugin_ui_phase_name(phase) : "none";
}

void plugin_ui_phase_node_clear_stack(plugin_ui_phase_node_t phase_node) {
    while(!TAILQ_EMPTY(&phase_node->m_state_stack)) {
        plugin_ui_state_node_free(TAILQ_LAST(&phase_node->m_state_stack, plugin_ui_state_node_list));
    }
}

void plugin_ui_phase_node_suspend_stack(plugin_ui_phase_node_t phase_node) {
    plugin_ui_state_node_t state_node;

    TAILQ_FOREACH_REVERSE(state_node, &phase_node->m_state_stack, plugin_ui_state_node_list, m_next) {
        plugin_ui_state_node_suspend(state_node);
        
        if (state_node->m_curent.m_inout_packages) {
            plugin_package_group_set_using_state(state_node->m_curent.m_inout_packages, plugin_package_package_using_state_free);
            phase_node->m_env->m_package_need_gc = 1;
        }

        if (state_node->m_curent.m_runtime_packages) {
            plugin_package_group_set_using_state(state_node->m_curent.m_runtime_packages, plugin_package_package_using_state_free);
            phase_node->m_env->m_package_need_gc = 1;
        }
        
        state_node->m_state = plugin_ui_state_node_state_suspend;

        if (phase_node->m_env->m_debug) {
            CPE_INFO(
                phase_node->m_env->m_module->m_em, "ui_env: " FMT_UINT32_T ": phase %s: state %s ==> %s",
                phase_node->m_env->m_tick, plugin_ui_phase_node_name(phase_node),
                plugin_ui_state_node_name(state_node), plugin_ui_state_node_state_str(state_node));
        }
    }
}

static plugin_ui_phase_node_t plugin_ui_env_phase_node_next(struct plugin_ui_phase_node_it * it) {
    plugin_ui_phase_node_t * data = (plugin_ui_phase_node_t *)(it->m_data);
    plugin_ui_phase_node_t r;
    if (*data == NULL) return NULL;
    r = *data;
    *data = TAILQ_NEXT(r, m_next);
    return r;
}

void plugin_ui_env_phase_nodes(plugin_ui_env_t env, plugin_ui_phase_node_it_t phase_node_it) {
    *(plugin_ui_phase_node_t *)(phase_node_it->m_data) = TAILQ_FIRST(&env->m_phase_stack);
    phase_node_it->next = plugin_ui_env_phase_node_next;
}

const char * plugin_ui_phase_node_op_to_str(enum plugin_ui_phase_node_op op) {
    switch(op) {
    case plugin_ui_phase_node_op_none:
        return "none";
    case plugin_ui_phase_node_op_init:
        return "init";
    case plugin_ui_phase_node_op_enter:
        return "enter";
    case plugin_ui_phase_node_op_suspend:
        return "suspend";
    case plugin_ui_phase_node_op_resume:
        return "resume";
    case plugin_ui_phase_node_op_remove:
        return "remove";
    case plugin_ui_phase_node_op_back:
        return "back";
    default:
        return "unknown";
    }
}

const char * plugin_ui_phase_node_state_to_str(plugin_ui_phase_node_state_t phase_node_state) {
    switch(phase_node_state) {
    case plugin_ui_phase_node_state_prepare_loading:
        return "prepare-loading";
    case plugin_ui_phase_node_state_loading:
        return "loading";
    case plugin_ui_phase_node_state_prepare_back:
        return "prepare_back";
    case plugin_ui_phase_node_state_back:
        return "back";
    case plugin_ui_phase_node_state_prepare_resume:
        return "prepare_resume";
    case plugin_ui_phase_node_state_processing:
        return "processing";
    case plugin_ui_phase_node_state_suspend:
        return "suspend";
    default:
        return "unknown";
    }
}

void plugin_ui_phase_node_print(write_stream_t s, plugin_ui_phase_node_t phase_node) {
    plugin_ui_state_node_t state_node;
    
    stream_printf(s, "%s[%s]: ", plugin_ui_phase_node_name(phase_node), plugin_ui_phase_node_state_str(phase_node));

    TAILQ_FOREACH(state_node, &phase_node->m_state_stack, m_next) {
        if (state_node != TAILQ_FIRST(&phase_node->m_state_stack)) {
            stream_printf(s, " ==> ");
        }
        stream_printf(s, "%s[%s", plugin_ui_state_node_name(state_node), plugin_ui_state_node_state_str(state_node));

        if (state_node->m_op != plugin_ui_state_node_op_none) {
            stream_printf(s, "+%s", plugin_ui_state_node_op_str(state_node));
        }

        if (state_node->m_replace.m_process_state) {
            stream_printf(s, "(%s)", plugin_ui_state_name(state_node->m_replace.m_process_state));
        }
        
        stream_printf(s, "]");
    }
}

const char * plugin_ui_phase_node_dump(mem_buffer_t buffer, plugin_ui_phase_node_t phase_node) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

    mem_buffer_clear_data(buffer);

    plugin_ui_phase_node_print((write_stream_t)&stream, phase_node);

    stream_putc((write_stream_t)&stream, 0);

    return (const char *)mem_buffer_make_continuous(buffer, 0);
}

void plugin_ui_phase_node_stop_package_load_task(plugin_ui_phase_node_t phase_node) {
    if (phase_node->m_package_load_task) {
        plugin_package_load_task_t task = plugin_package_load_task_find_by_id(phase_node->m_env->m_module->m_package_module, phase_node->m_package_load_task);
        if (task) {
            plugin_package_load_task_free(task);
        }
        phase_node->m_package_load_task = 0;
    }
}

uint8_t plugin_ui_phase_node_check_package_load_task_runing(plugin_ui_phase_node_t phase_node) {
    plugin_package_load_task_t task;
    
    if (phase_node->m_package_load_task == 0) return 0;
    
    task = plugin_package_load_task_find_by_id(phase_node->m_env->m_module->m_package_module, phase_node->m_package_load_task);
    if (task == NULL) {
        phase_node->m_package_load_task = 0;
        return 0;
    }
    else {
        return 1;
    }
}

plugin_package_load_task_t
plugin_ui_phase_node_create_package_load_task(plugin_ui_phase_node_t phase_node) {
    plugin_package_load_task_t task;
    
    assert(phase_node->m_package_load_task == 0);

    task = plugin_package_load_task_create(phase_node->m_env->m_module->m_package_module, phase_node, NULL, 0);
    if (task == NULL) {
        CPE_ERROR(phase_node->m_env->m_module->m_em, "plugin_ui_phase_node_create_package_load_task: create fail!");
        return NULL;
    }

    phase_node->m_package_load_task = plugin_package_load_task_id(task);
    return task;
}
