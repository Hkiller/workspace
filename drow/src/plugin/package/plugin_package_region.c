#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/pal/pal_stdio.h"
#include "plugin_package_region_i.h"
#include "plugin_package_group_i.h"

plugin_package_region_t
plugin_package_region_create(plugin_package_module_t module, const char * name) {
    plugin_package_region_t region;
    char group_name[64];
    
    region = mem_alloc(module->m_alloc, sizeof(struct plugin_package_region));
    if (region == NULL) {
        CPE_ERROR(module->m_em, "%s: region: alloc fail!", plugin_package_module_name(module));
        return NULL;
    }

    snprintf(group_name, sizeof(group_name), "region-%s", name);
    region->m_module = module;
    cpe_str_dup(region->m_name, sizeof(region->m_name), name);
    region->m_group = plugin_package_group_create(module, group_name);
    if (region->m_group == NULL) {
        CPE_ERROR(module->m_em, "%s: region: create group fail!", plugin_package_module_name(module));
        mem_free(module->m_alloc, region);
        return NULL;
    }
    
    TAILQ_INSERT_TAIL(&module->m_regions, region, m_next_for_module);

    return region;
}

void plugin_package_region_free(plugin_package_region_t region) {
    plugin_package_module_t module = region->m_module;

    plugin_package_group_free(region->m_group);

    TAILQ_REMOVE(&module->m_regions, region, m_next_for_module);

    mem_free(module->m_alloc, region);
}

plugin_package_module_t plugin_package_region_module(plugin_package_region_t region) {
    return region->m_module;
}

plugin_package_group_t plugin_package_region_group(plugin_package_region_t region) {
    return region->m_group;
}

const char * plugin_package_region_name(plugin_package_region_t region) {
    return region->m_name;
}

plugin_package_region_t plugin_package_region_find(plugin_package_module_t module, const char * name) {
    plugin_package_region_t region;

    TAILQ_FOREACH(region, &module->m_regions, m_next_for_module) {
        if (strcmp(region->m_name, name) == 0) return region;
    }

    return NULL;
}

static plugin_package_region_t plugin_package_module_region_it_do_next(plugin_package_region_it_t region_it) {
    plugin_package_region_t * data = (plugin_package_region_t*)region_it->m_data;
    plugin_package_region_t r = *data;

    if (r) *data = TAILQ_NEXT(r, m_next_for_module);

    return r;
}

void plugin_package_module_regions(plugin_package_module_t module, plugin_package_region_it_t region_it) {
    plugin_package_region_t * data = (plugin_package_region_t*)region_it->m_data;

    *data = TAILQ_FIRST(&module->m_regions);
    region_it->next = plugin_package_module_region_it_do_next;
}
