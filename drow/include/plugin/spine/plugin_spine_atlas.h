#ifndef PLUGIN_SPINE_DATA_ATLAS_H
#define PLUGIN_SPINE_DATA_ATLAS_H
#include "plugin_spine_types.h"
#include "spine/spine.h"

#ifdef __cplusplus
extern "C" {
#endif

/*spine*/
int plugin_spine_atlas_load_page_from_model(spAtlas * atlas, const char * name, ui_data_module_t module, ui_data_sprite_t sprite);    

spAtlas * plugin_spine_atlas_create(plugin_spine_module_t module);
void plugin_spine_atlas_free(spAtlas * atlas);

spAtlas * plugin_spine_atlas_clone(spAtlas * from_atlas);
spAtlas * plugin_spine_atlas_load_from_model(plugin_spine_module_t module, const char * path);
    
#ifdef __cplusplus
}
#endif

#endif
