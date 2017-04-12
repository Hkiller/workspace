#include "plugin_scrollmap_obj_type_map_i.h"

plugin_scrollmap_obj_type_map_t
plugin_scrollmap_obj_type_map_create_on_team(
    plugin_scrollmap_team_t team, const char * from_type, const char * to_type)
{
    plugin_scrollmap_obj_type_map_t obj_type_map;
    size_t from_type_size = strlen(from_type) + 1;
    size_t to_type_size = strlen(to_type) + 1;

    obj_type_map =
        mem_alloc(team->m_env->m_module->m_alloc, sizeof(struct plugin_scrollmap_obj_type_map) + from_type_size + to_type_size);
    if (obj_type_map == NULL) {
        CPE_ERROR(team->m_env->m_module->m_em, "plugin_scrollmap_obj_type_map_create_on_team: alloc fail!");
        return NULL;
    }

    obj_type_map->m_team = team;
    obj_type_map->m_from_type = (void*)(obj_type_map + 1);
    obj_type_map->m_to_type = obj_type_map->m_from_type + from_type_size;

    memcpy((void*)obj_type_map->m_from_type, from_type, from_type_size);
    memcpy((void*)obj_type_map->m_to_type, to_type, to_type_size);
    
    TAILQ_INSERT_TAIL(&team->m_obj_type_maps, obj_type_map, m_next);

    return obj_type_map;
}

void plugin_scrollmap_obj_type_map_free(plugin_scrollmap_obj_type_map_t obj_type_map) {
    TAILQ_REMOVE(&obj_type_map->m_team->m_obj_type_maps, obj_type_map, m_next);
    mem_free(obj_type_map->m_team->m_env->m_module->m_alloc, obj_type_map);
}

plugin_scrollmap_obj_type_map_t
plugin_scrollmap_obj_type_map_find_on_team(
    plugin_scrollmap_team_t team, const char * from_type)
{
    plugin_scrollmap_obj_type_map_t obj_type_map;

    TAILQ_FOREACH(obj_type_map, &team->m_obj_type_maps, m_next) {
        if (strcmp(obj_type_map->m_from_type, from_type) == 0) return obj_type_map;
    }

    return NULL;
}

