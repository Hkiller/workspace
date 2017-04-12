#ifndef PLUGIN_PACKAGE_MANIP_SEARCH_MAP_I_H
#define PLUGIN_PACKAGE_MANIP_SEARCH_MAP_I_H
#include "plugin_package_manip_search_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_package_manip_search_map {
    plugin_package_manip_search_t m_search;
    TAILQ_ENTRY(plugin_package_manip_search_map) m_next;
    char * m_def;
};

plugin_package_manip_search_map_t
plugin_package_manip_search_map_create(plugin_package_manip_search_t search);

void plugin_package_manip_search_map_free(plugin_package_manip_search_map_t map);

#ifdef __cplusplus
}
#endif

#endif 
