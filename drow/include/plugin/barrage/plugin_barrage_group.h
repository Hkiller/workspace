#ifndef DROW_PLUGIN_BARRAGE_GROUP_H
#define DROW_PLUGIN_BARRAGE_GROUP_H
#include "plugin_barrage_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_barrage_group_t plugin_barrage_group_create(plugin_barrage_env_t env, const char * name);
void plugin_barrage_group_free(plugin_barrage_group_t group);

plugin_barrage_group_t
plugin_barrage_group_find(plugin_barrage_env_t env, const char * name);

void plugin_barrage_group_bullets(plugin_barrage_bullet_it_t bullet_it, plugin_barrage_group_t group);
    
#ifdef __cplusplus
}
#endif

#endif

