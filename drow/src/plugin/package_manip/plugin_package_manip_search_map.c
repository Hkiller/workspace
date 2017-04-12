#include "cpe/utils/string_utils.h"
#include "plugin_package_manip_search_map_i.h"

plugin_package_manip_search_map_t
plugin_package_manip_search_map_create(plugin_package_manip_search_t search) {
    plugin_package_manip_search_map_t map;

    map = mem_alloc(search->m_manip->m_alloc, sizeof(struct plugin_package_manip_search_map));
    if (map == NULL) {
        CPE_ERROR(search->m_manip->m_em, "plugin_package_manip_search_map_create: alloc fail!");
        return NULL;
    }

    map->m_search = search;
    map->m_def = NULL;

    TAILQ_INSERT_TAIL(&search->m_maps, map, m_next);

    if (search->m_cur_map == NULL) {
        search->m_cur_map = map;
    }
    
    return map;
}

void plugin_package_manip_search_map_free(plugin_package_manip_search_map_t map) {
    plugin_package_manip_search_t search = map->m_search;

    mem_free(search->m_manip->m_alloc, map->m_def);
    
    TAILQ_REMOVE(&search->m_maps, map, m_next);

    if (search->m_cur_map == map) {
        search->m_cur_map = TAILQ_FIRST(&search->m_maps);
    }
    
    mem_free(search->m_manip->m_alloc, map);
}

