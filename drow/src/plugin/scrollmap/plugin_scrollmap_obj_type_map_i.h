#ifndef PLUGIN_SCROLLMAP_OBJ_TYPE_MAP_I_H
#define PLUGIN_SCROLLMAP_OBJ_TYPE_MAP_I_H
#include "plugin/scrollmap/plugin_scrollmap_obj_type_map.h"
#include "plugin_scrollmap_team_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_scrollmap_obj_type_map {
    plugin_scrollmap_team_t m_team;
    TAILQ_ENTRY(plugin_scrollmap_obj_type_map) m_next;
    const char * m_from_type;
    const char * m_to_type;
};
    
#ifdef __cplusplus
}
#endif

#endif
