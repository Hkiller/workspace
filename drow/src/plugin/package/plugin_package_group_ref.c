#include <assert.h>
#include "plugin_package_group_ref_i.h"
#include "plugin_package_group_i.h"
#include "plugin_package_package_i.h"

plugin_package_group_ref_t
plugin_package_group_ref_create(plugin_package_group_t group, plugin_package_package_t package) {
    plugin_package_group_ref_t ref;
    plugin_package_module_t module = group->m_module;

    ref = TAILQ_FIRST(&module->m_free_group_refs);
    if (ref) {
        TAILQ_REMOVE(&module->m_free_group_refs, ref, m_next_for_group);
    }
    else {
        ref = mem_alloc(module->m_alloc, sizeof(struct plugin_package_group_ref));
        if (ref == NULL) {
            CPE_ERROR(module->m_em, "plugin_package_group_ref alloc fail!");
            return NULL;
        }
    }
    
    ref->m_group = group;
    ref->m_package = package;

    group->m_package_count++;

    TAILQ_INSERT_TAIL(&group->m_packages, ref, m_next_for_group);
    TAILQ_INSERT_TAIL(&package->m_groups, ref, m_next_for_package);

    return ref;
}

void plugin_package_group_ref_free(plugin_package_group_ref_t ref) {
    plugin_package_module_t module = ref->m_group->m_module;

    assert(ref->m_group->m_package_count > 0);
    ref->m_group->m_package_count--;
    TAILQ_REMOVE(&ref->m_group->m_packages, ref, m_next_for_group);
    TAILQ_REMOVE(&ref->m_package->m_groups, ref, m_next_for_package);

    ref->m_group = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_group_refs, ref, m_next_for_group);
}

void plugin_package_group_ref_real_free(plugin_package_group_ref_t ref) {
    plugin_package_module_t module = (void*)ref->m_group;
    
    TAILQ_REMOVE(&module->m_free_group_refs, ref, m_next_for_group);
    mem_free(module->m_alloc, ref);
}
