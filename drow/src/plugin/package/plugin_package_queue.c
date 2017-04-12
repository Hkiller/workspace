#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/pal/pal_stdio.h"
#include "plugin_package_queue_i.h"
#include "plugin_package_package_i.h"
#include "plugin_package_group_i.h"
#include "plugin_package_group_ref_i.h"

plugin_package_queue_t
plugin_package_queue_create(plugin_package_module_t module, const char * name, plugin_package_queue_policy_t policy) {
    plugin_package_queue_t queue;
    size_t name_len = strlen(name) + 1;
    char group_name[64];

    snprintf(group_name, sizeof(group_name), "queue-%s", name);
    
    queue = mem_alloc(module->m_alloc, sizeof(struct plugin_package_queue) + name_len);
    if (queue == NULL) {
        CPE_ERROR(module->m_em, "plugin_package_queue_create: package create fail!");
        return NULL;
    }

    memcpy(queue + 1, name, name_len);
    
    queue->m_module = module;
    queue->m_policy = policy;
    queue->m_name = (void*)(queue + 1);
    queue->m_limit = (uint32_t)-1;
    queue->m_group = plugin_package_group_create(module, group_name);
    if (queue->m_group == NULL) {
        CPE_ERROR(module->m_em, "plugin_package_queue_create: create group fail!");
        return NULL;
    }
    plugin_package_group_set_using_state(queue->m_group, plugin_package_package_using_state_ref_count);
    
    cpe_hash_entry_init(&queue->m_hh);
    if (cpe_hash_table_insert_unique(&module->m_queues, queue) != 0) {
        CPE_ERROR(module->m_em, "plugin_package_queue_create: package %s duplicate!", name);
        plugin_package_group_free(queue->m_group);
        mem_free(module->m_alloc, queue);
        return NULL;
    }

    return queue;
}

void plugin_package_queue_free(plugin_package_queue_t queue) {
    plugin_package_module_t module = queue->m_module;

    plugin_package_group_free(queue->m_group);
    
    cpe_hash_table_remove_by_ins(&module->m_queues, queue);

    mem_free(module->m_alloc, queue);
}

void plugin_package_queue_free_all(plugin_package_module_t module) {
    struct cpe_hash_it queue_it;
    plugin_package_queue_t queue;

    cpe_hash_it_init(&queue_it, &module->m_queues);

    queue = cpe_hash_it_next(&queue_it);
    while (queue) {
        plugin_package_queue_t next = cpe_hash_it_next(&queue_it);
        plugin_package_queue_free(queue);
        queue = next;
    }
}

plugin_package_queue_t
plugin_package_queue_find(plugin_package_module_t module, const char * name) {
    struct plugin_package_queue key;
    key.m_name = name;
    return cpe_hash_table_find(&module->m_queues, &key);
}

void plugin_package_queue_clear(plugin_package_queue_t queue) {
    plugin_package_group_clear(queue->m_group);
}

const char * plugin_package_queue_name(plugin_package_queue_t queue) {
    return queue->m_name;
}

uint32_t plugin_package_queue_limit(plugin_package_queue_t queue) {
    return queue->m_limit;
}

void plugin_package_queue_set_limit(plugin_package_queue_t queue, uint32_t limit) {
    if (queue->m_limit == limit) return;
    
    queue->m_limit = limit;

    if (queue->m_limit != ((uint32_t)-1) && queue->m_limit < plugin_package_group_package_count(queue->m_group)) {
        switch(queue->m_policy) {
        case plugin_package_queue_policy_manual:
            CPE_ERROR(
                queue->m_module->m_em, "package queue %s: set limit %d, package count %d is already overflow!" ,
                queue->m_name, limit, plugin_package_group_package_count(queue->m_group));
            break;
        case plugin_package_queue_policy_lru:
            while(queue->m_group->m_package_count> queue->m_limit) {
                plugin_package_package_t package = TAILQ_LAST(&queue->m_group->m_packages, plugin_package_package_list);
                assert(package);
                plugin_package_queue_remove_package(queue, package);
                if (plugin_package_package_using_state(package) < plugin_package_package_using_state_ref_count) {
                    plugin_package_package_unload(package);
                }
            }
            break;
        default:
            break;
        }
    }
    
    if (queue->m_module->m_debug) {
        CPE_INFO(queue->m_module->m_em, "packag queue %s: limit ==> %d", queue->m_name, (int32_t)queue->m_limit);
    }
}

uint32_t plugin_package_queue_package_count(plugin_package_queue_t queue) {
    return queue->m_group->m_package_count;
}

void plugin_package_queue_remove_package(plugin_package_queue_t queue, plugin_package_package_t package) {
    plugin_package_group_remove_package(queue->m_group, package);
}

int plugin_package_queue_add_package(plugin_package_queue_t queue, plugin_package_package_t package) {
    if (queue->m_limit != ((uint32_t)-1)) {
        switch(queue->m_policy) {
        case plugin_package_queue_policy_manual:
            if (queue->m_group->m_package_count + 1 > queue->m_limit) {
                CPE_ERROR(
                    queue->m_module->m_em, "package queue %s: package count overflow, limit=%d!" ,
                    queue->m_name, queue->m_limit);
                return -1;
            }
            break;
        case plugin_package_queue_policy_lru:
            break;
            if (queue->m_limit == 0) {
                CPE_ERROR(
                    queue->m_module->m_em, "package queue %s: limit 0, can`t add package!" ,
                    queue->m_name);
                return -1;
            }
            else {
                while(queue->m_group->m_package_count + 1 > queue->m_limit) {
                    plugin_package_package_t package = TAILQ_LAST(&queue->m_group->m_packages, plugin_package_package_list);
                    assert(package);
                    plugin_package_group_remove_package(queue->m_group, package);
                    if (plugin_package_package_using_state(package) < plugin_package_package_using_state_ref_count) {
                        plugin_package_package_unload(package);
                    }
                }
            }
            break;
        default:
            CPE_ERROR(
                queue->m_module->m_em, "package queue %s: policy %d unknown!", 
                queue->m_name, queue->m_policy);
            return -1;
        }
    }

    return plugin_package_group_add_package(queue->m_group, package);
}

int plugin_package_queue_add_packages(plugin_package_queue_t queue, plugin_package_group_t group) {
    int rv = 0;
    plugin_package_group_ref_t ref;

    TAILQ_FOREACH(ref, &group->m_packages, m_next_for_group) {
        if (plugin_package_queue_add_package(queue, ref->m_package) != 0) rv = -1;
    }

    return rv;
}

int plugin_package_queue_load_all_async(plugin_package_queue_t queue, plugin_package_load_task_t task) {
    return plugin_package_group_load_async(queue->m_group, task);
}

uint32_t plugin_package_queue_hash(plugin_package_queue_t package) {
    return cpe_hash_str(package->m_name, strlen(package->m_name));
}

int plugin_package_queue_eq(plugin_package_queue_t l, plugin_package_queue_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

static plugin_package_queue_t plugin_package_queue_it_do_next(plugin_package_queue_it_t it) {
    struct cpe_hash_it * hash_it = (void*)it->m_data;
    return cpe_hash_it_next(hash_it);    
}

void plugin_package_queues(plugin_package_queue_it_t package_it, plugin_package_module_t module) {
    struct cpe_hash_it * hash_it = (void*)package_it->m_data;
    
    assert(CPE_ARRAY_SIZE(package_it->m_data) >= sizeof(struct cpe_hash_it));

    cpe_hash_it_init(hash_it, &module->m_packages);
    package_it->next = plugin_package_queue_it_do_next;
}
