#include "cpe/utils/string_utils.h"
#include "plugin_package_manip_res_collector_i.h"

plugin_package_manip_res_collector_t
plugin_package_manip_res_collector_create(
    plugin_package_manip_t manip, const char * name, plugin_package_manip_res_collector_fun_t fun, void * ctx)
{
    plugin_package_manip_res_collector_t collector;

    collector = mem_alloc(manip->m_alloc, sizeof(struct plugin_package_manip_res_collector));
    if (collector == NULL) {
        CPE_ERROR(manip->m_em, "plugin_package_manip_res_collector_create: alloc fail!");
        return NULL;
    }

    collector->m_manip = manip;
    cpe_str_dup(collector->m_name, sizeof(collector->m_name), name);
    collector->m_fun = fun;
    collector->m_ctx = ctx;

    TAILQ_INSERT_TAIL(&manip->m_res_collectors, collector, m_next);
    
    return collector;
}

void plugin_package_manip_res_collector_free(plugin_package_manip_res_collector_t collector) {
    plugin_package_manip_t manip = collector->m_manip;

    TAILQ_REMOVE(&manip->m_res_collectors, collector, m_next);

    mem_free(manip->m_alloc, collector);
}

plugin_package_manip_res_collector_t
plugin_package_manip_res_collector_find(plugin_package_manip_t manip, const char * name) {
    plugin_package_manip_res_collector_t collector;

    TAILQ_FOREACH(collector, &manip->m_res_collectors, m_next) {
        if (strcmp(collector->m_name, name) == 0) return collector;
    }

    return NULL;
}
