#ifndef PLUGIN_SCROLLMAP_OBJ_TYPE_MAP_H
#define PLUGIN_SCROLLMAP_OBJ_TYPE_MAP_H
#include "plugin_scrollmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_scrollmap_obj_type_map_t
plugin_scrollmap_obj_type_map_create_on_team(
    plugin_scrollmap_team_t team, const char * from_type, const char * to_type);

void plugin_scrollmap_obj_type_map_free(plugin_scrollmap_obj_type_map_t obj_type_map);

plugin_scrollmap_obj_type_map_t
plugin_scrollmap_obj_type_map_find_on_team(
    plugin_scrollmap_team_t team, const char * from_type);
    
#ifdef __cplusplus
}
#endif

#endif
