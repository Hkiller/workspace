#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin_package_group_i.h"
#include "plugin_package_package_i.h"
#include "plugin_package_depend_i.h"
#include "plugin_package_group_ref_i.h"

plugin_package_group_t
plugin_package_group_create(plugin_package_module_t module, const char * name) {
    plugin_package_group_t group;

    group = TAILQ_FIRST(&module->m_free_groups);
    if (group) {
        TAILQ_REMOVE(&module->m_free_groups, group, m_next_for_module);
    }
    else {
        group = mem_alloc(module->m_alloc, sizeof(struct plugin_package_group));
        if (group == NULL) {
            CPE_ERROR(module->m_em, "%s: group: alloc fail!", plugin_package_module_name(module));
            return NULL;
        }
    }
    
    group->m_module = module;
    cpe_str_dup(group->m_name, sizeof(group->m_name), name);
    group->m_package_using_state = plugin_package_package_using_state_free;
    group->m_package_count = 0;
    TAILQ_INIT(&group->m_packages);

    TAILQ_INSERT_TAIL(&module->m_groups, group, m_next_for_module);

    return group;
}

void plugin_package_group_free(plugin_package_group_t group) {
    plugin_package_module_t module = group->m_module;

    while(!TAILQ_EMPTY(&group->m_packages)) {
        plugin_package_group_ref_free(TAILQ_FIRST(&group->m_packages));
    }
    assert(group->m_package_count == 0);

    TAILQ_REMOVE(&module->m_groups, group, m_next_for_module);
    TAILQ_INSERT_TAIL(&module->m_free_groups, group, m_next_for_module);    
}

void plugin_package_group_real_free(plugin_package_group_t group) {
    TAILQ_REMOVE(&group->m_module->m_free_groups, group, m_next_for_module);    
    mem_free(group->m_module->m_alloc, group);
}

plugin_package_module_t plugin_package_group_module(plugin_package_group_t group) {
    return group->m_module;
}

const char * plugin_package_group_name(plugin_package_group_t group) {
    return group->m_name;
}

void plugin_package_group_set_name(plugin_package_group_t group, const char * name) {
    cpe_str_dup(group->m_name, sizeof(group->m_name), name);
}

plugin_package_package_using_state_t plugin_package_group_using_state(plugin_package_group_t group) {
    return group->m_package_using_state;
}

void plugin_package_group_set_using_state(plugin_package_group_t group, plugin_package_package_using_state_t using_state) {
    group->m_package_using_state = using_state;
}

void plugin_package_group_clear(plugin_package_group_t group) {
    while(!TAILQ_EMPTY(&group->m_packages)) {
        plugin_package_group_ref_free(TAILQ_FIRST(&group->m_packages));
    }
}

uint32_t plugin_package_group_package_count(plugin_package_group_t group) {
    return group->m_package_count;
}

void plugin_package_group_remove_package(plugin_package_group_t group, plugin_package_package_t package) {
    plugin_package_group_ref_t ref;

    TAILQ_FOREACH(ref, &package->m_groups, m_next_for_package) {
        if (ref->m_group == group) {
            plugin_package_group_ref_free(ref);
            break;
        }
    }
}

int plugin_package_group_add_package(plugin_package_group_t group, plugin_package_package_t package) {
    plugin_package_group_ref_t ref;

    TAILQ_FOREACH(ref, &package->m_groups, m_next_for_package) {
        if (ref->m_group == group) return 0;
    }

    ref = plugin_package_group_ref_create(group, package);
    return ref ? 0 : -1;
}

int plugin_package_group_add_packages(plugin_package_group_t group, plugin_package_group_t from_group) {
    plugin_package_group_ref_t ref;
    int rv = 0;
    
    TAILQ_FOREACH(ref, &from_group->m_packages, m_next_for_group) {
        if (plugin_package_group_add_package(group, ref->m_package) != 0) rv = -1;
    }

    return rv;
}

int plugin_package_group_add_package_r(plugin_package_group_t group, plugin_package_package_t package) {
    int rv = 0;
    plugin_package_depend_t dep;

    if (plugin_package_package_is_in_group(package, group)) return 0;
    if (plugin_package_group_add_package(group, package) != 0) return -1;

    TAILQ_FOREACH(dep, &package->m_base_packages, m_next_for_extern) {
        if (plugin_package_group_add_package_r(group, dep->m_base_package) != 0) rv = -1;
    }

    return rv;
}

int plugin_package_group_add_base_packages_r(plugin_package_group_t group, plugin_package_package_t package) {
    int rv = 0;
    plugin_package_depend_t dep;

    TAILQ_FOREACH(dep, &package->m_base_packages, m_next_for_extern) {
        if (plugin_package_group_add_package_r(group, dep->m_base_package) != 0) rv = -1;
    }

    return rv;
}

int plugin_package_group_add_packages_r(plugin_package_group_t group, plugin_package_group_t from_group) {
    plugin_package_group_ref_t ref;
    int rv = 0;
    
    TAILQ_FOREACH(ref, &from_group->m_packages, m_next_for_group) {
        if (plugin_package_group_add_package_r(group, ref->m_package) != 0) rv = -1;
    }

    return rv;
}

static plugin_package_package_t plugin_package_group_package_next(struct plugin_package_package_it * it) {
    plugin_package_group_ref_t * data = (plugin_package_group_ref_t *)(it->m_data);
    plugin_package_group_ref_t r;
    if (*data == NULL) return NULL;
    r = *data;
    *data = TAILQ_NEXT(r, m_next_for_group);
    return r->m_package;
}

void plugin_package_group_packages(plugin_package_package_it_t it, plugin_package_group_t group) {
    plugin_package_group_ref_t * data = (plugin_package_group_ref_t *)(it->m_data);
    *data = TAILQ_FIRST(&group->m_packages);
    it->next = plugin_package_group_package_next;
}

int plugin_package_group_load_sync(plugin_package_group_t group) {
    int rv = 0;
    plugin_package_group_ref_t ref;

    TAILQ_FOREACH(ref, &group->m_packages, m_next_for_group) {
        if (plugin_package_package_load_sync(ref->m_package) != 0) rv = -1;
    }

    return rv;
}

int plugin_package_group_load_async(plugin_package_group_t group, plugin_package_load_task_t task) {
    int rv = 0;
    plugin_package_group_ref_t ref;

    TAILQ_FOREACH(ref, &group->m_packages, m_next_for_group) {
        if (plugin_package_package_load_async(ref->m_package, task) != 0) rv = -1;
    }

    return rv;
}

int plugin_package_group_expand_base_packages(plugin_package_group_t group) {
    plugin_package_group_ref_t ref;
    int rv = 0;

    for(ref = TAILQ_FIRST(&group->m_packages); ref; ref = TAILQ_NEXT(ref, m_next_for_group)) {
        struct plugin_package_package_it base_package_it;
        plugin_package_package_t base_package;
        
        plugin_package_package_base_packages(ref->m_package, &base_package_it);
        while((base_package = plugin_package_package_it_next(&base_package_it))) {
            if (plugin_package_group_add_package(group, base_package) != 0) rv = -1;
        }
    }

    return rv;
}
