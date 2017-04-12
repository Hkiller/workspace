#ifndef PLUGIN_SPINE_OBJ_TRACK_H
#define PLUGIN_SPINE_OBJ_TRACK_H
#include "spine/AnimationState.h"
#include "plugin_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif
        
plugin_spine_obj_track_t plugin_spine_obj_track_create(plugin_spine_obj_t obj, const char * track_name);
void plugin_spine_obj_track_free(plugin_spine_obj_track_t track);

plugin_spine_obj_track_t plugin_spine_obj_track_find(plugin_spine_obj_t obj, const char * track_name);

float plugin_spine_obj_track_time_scale(plugin_spine_obj_track_t track);
void plugin_spine_obj_track_set_time_scale(plugin_spine_obj_track_t track, float time_scale);
    
#ifdef __cplusplus
}
#endif

#endif
