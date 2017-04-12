#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "cpe/pal/pal_strings.h"
#include "plugin_package_load_task_i.h"
#include "plugin_package_package_i.h"
#include "plugin_package_depend_i.h"
#include "plugin_package_group_i.h"

plugin_package_load_task_t
plugin_package_load_task_create(plugin_package_module_t module, void * ctx, plugin_package_load_task_progress_fun_t fun, size_t carry_data_size) {
    plugin_package_load_task_t task;

    if (carry_data_size > CPE_TYPE_ARRAY_SIZE(struct plugin_package_load_task, m_carry_data)) {
        CPE_ERROR(module->m_em, "%s: task: carry data size %d overflow!", plugin_package_module_name(module), (int)carry_data_size);
        return NULL;
    }
    
    task = TAILQ_FIRST(&module->m_free_load_tasks);
    if (task) {
        TAILQ_REMOVE(&module->m_free_load_tasks, task, m_next_for_module);
    }
    else {
        task = mem_alloc(module->m_alloc, sizeof(struct plugin_package_load_task));
        if (task == NULL) {
            CPE_ERROR(module->m_em, "%s: task: alloc fail!", plugin_package_module_name(module));
            return NULL;
        }
    }
    
    task->m_id = ++module->m_max_load_task_id;
    task->m_module = module;
    task->m_ctx = ctx;
    task->m_process_fun = fun;
    task->m_process = 0.0f; 
    task->m_total_package_count = 0;
    task->m_left_package_count = 0;
    task->m_left_package_count = 0;
    bzero(task->m_carry_data, sizeof(task->m_carry_data));
    task->m_carry_data_size = (uint8_t)carry_data_size;

    task->m_group = plugin_package_group_create(module, "");
    if (task->m_group == NULL) {
        CPE_ERROR(module->m_em, "%s: task: create group fail!", plugin_package_module_name(module));
        TAILQ_INSERT_TAIL(&module->m_free_load_tasks, task, m_next_for_module);
        return NULL;
    }
    
    TAILQ_INSERT_TAIL(&module->m_load_tasks, task, m_next_for_module);

    return task;
}

void plugin_package_load_task_free(plugin_package_load_task_t task) {
    plugin_package_module_t module = task->m_module;

    TAILQ_REMOVE(&module->m_load_tasks, task, m_next_for_module);
    TAILQ_INSERT_TAIL(&module->m_free_load_tasks, task, m_next_for_module);    
}

void plugin_package_load_task_real_free(plugin_package_load_task_t task) {
    TAILQ_REMOVE(&task->m_module->m_free_load_tasks, task, m_next_for_module);
    mem_free(task->m_module->m_alloc, task);
}

plugin_package_module_t plugin_package_load_task_module(plugin_package_load_task_t task) {
    return task->m_module;
}

int plugin_package_load_task_add_package(plugin_package_load_task_t task, plugin_package_package_t package) {
    uint32_t old_package_count;
    int rv;
    uint32_t add_package_count;

    if (plugin_package_package_state(package) == plugin_package_package_loaded) return 0;
    
    old_package_count = task->m_group->m_package_count;
    rv = plugin_package_group_add_package(task->m_group, package);
    add_package_count = task->m_group->m_package_count - old_package_count;
    task->m_total_package_count += add_package_count;
    task->m_left_package_count += add_package_count;
    
    return rv;
}

int plugin_package_load_task_add_package_r(plugin_package_load_task_t task, plugin_package_package_t package) {
    int rv = 0;
    plugin_package_depend_t dep;
    
    if (plugin_package_load_task_add_package(task, package) != 0) rv = -1;

    TAILQ_FOREACH(dep, &package->m_base_packages, m_next_for_extern) {
        if (plugin_package_load_task_add_package_r(task, dep->m_base_package) != 0) rv = -1;
    }
    
    return rv;
}

uint32_t plugin_package_load_task_id(plugin_package_load_task_t task) {
    return task->m_id;
}

void * plugin_package_load_task_carry_data(plugin_package_load_task_t task, size_t size) {
    assert(size == task->m_carry_data_size);
    return task->m_carry_data;
}

plugin_package_load_task_t
plugin_package_load_task_find_by_id(plugin_package_module_t module, uint32_t id) {
    plugin_package_load_task_t task;

    TAILQ_FOREACH(task, &module->m_load_tasks, m_next_for_module) {
        if (task->m_id == id) {
            return task->m_left_package_count == 0 ? NULL : task;
        }
    }

    return NULL;
}

uint32_t plugin_package_load_task_total_download_count(plugin_package_load_task_t task) {
    return task->m_total_download_count;
}

uint32_t plugin_package_load_task_total_package_count(plugin_package_load_task_t task) {
    return task->m_total_package_count;
}

void plugin_package_load_task_packages(plugin_package_load_task_t task, plugin_package_package_it_t it) {
    plugin_package_group_packages(it, task->m_group);
}

void plugin_package_load_task_tick(plugin_package_load_task_t task) {
    struct plugin_package_package_it package_it;
    plugin_package_package_t package;
    float install_progress = 0.0f;

    task->m_left_package_count = 0;
    
    plugin_package_group_packages(&package_it, task->m_group);
    while((package = plugin_package_package_it_next(&package_it))) {
        switch(package->m_state) {
        case plugin_package_package_empty:
        case plugin_package_package_downloading:
        case plugin_package_package_installed:
        case plugin_package_package_loading:
            task->m_left_package_count++;
            break;
        case plugin_package_package_loaded:
            install_progress += 1.0f / (float)task->m_total_package_count;
            break;
        default:
            break;
        }
    }

    if (task->m_left_package_count == 0) {
        task->m_process = 1.0f;
        if (task->m_process_fun) {
            task->m_process_fun(task->m_ctx, task, NULL, 1.0f);
        }
        plugin_package_load_task_free(task);
    }
    else {
        if (fabs(task->m_process - install_progress) > 0.01) {
            task->m_process = install_progress;
            if (task->m_process_fun) {
                task->m_process_fun(task->m_ctx, task, NULL, task->m_process);
            }
        }
    }
}

void plugin_package_load_task_free_by_ctx(plugin_package_module_t module, void * ctx) {
    plugin_package_load_task_t task, next_task;

    for(task = TAILQ_FIRST(&module->m_load_tasks); task; task = next_task) {
        next_task = TAILQ_NEXT(task, m_next_for_module);
        if (task->m_ctx == ctx) {
            plugin_package_load_task_free(task);
        }
    }
}
