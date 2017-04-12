#ifndef DROW_PLUGIN_BARRAGE_RENDER_OBJ_H
#define DROW_PLUGIN_BARRAGE_RENDER_OBJ_H
#include "plugin_barrage_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void plugin_barrage_render_set_env(plugin_barrage_render_t render, plugin_barrage_env_t env);
void plugin_barrage_render_set_group(plugin_barrage_render_t render, const char * group_name);
    
#ifdef __cplusplus
}
#endif

#endif

