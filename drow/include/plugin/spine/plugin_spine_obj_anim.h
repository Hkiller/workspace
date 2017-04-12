#ifndef PLUGIN_SPINE_OBJ_ANIM_H
#define PLUGIN_SPINE_OBJ_ANIM_H
#include "render/model/ui_model_types.h"
#include "spine/AnimationState.h"
#include "plugin_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif
        
plugin_spine_obj_anim_t
plugin_spine_obj_anim_create_in_track(
    plugin_spine_obj_track_t track, const char * anim_name, uint16_t loop_count);
void plugin_spine_obj_anim_free(plugin_spine_obj_anim_t anim);

plugin_spine_obj_anim_t
plugin_spine_obj_anim_create_in_obj(
    plugin_spine_obj_t obj, const char * track_name, const char * anim_name, uint16_t loop_count);

plugin_spine_obj_anim_t
plugin_spine_obj_anim_create_from_def(plugin_spine_obj_t obj, const char * anim_def);

plugin_spine_obj_track_t plugin_spine_obj_anim_track(plugin_spine_obj_anim_t anim);

int plugin_spine_obj_play_anims(plugin_spine_obj_t obj, const char * defs, plugin_spine_obj_anim_group_t group);
uint8_t plugin_spine_obj_anim_is_playing(plugin_spine_obj_anim_t anim);
    
int plugin_spine_obj_anim_add_track_listener(plugin_spine_obj_anim_t anim, plugin_spine_anim_event_fun_t fun, void * ctx);
void plugin_spine_obj_anim_remove_track_listener(plugin_spine_obj_anim_t anim, void * ctx);

int plugin_spine_obj_add_listener(plugin_spine_obj_t obj, plugin_spine_anim_event_fun_t fun, void * ctx);
void plugin_spine_obj_remove_listener(plugin_spine_obj_t obj, void * ctx);

int plugin_spine_obj_track_play_anims(plugin_spine_obj_track_t track, const char * defs, plugin_spine_obj_anim_group_t group);
    
#ifdef __cplusplus
}
#endif

#endif
