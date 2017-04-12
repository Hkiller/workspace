#ifndef PLUGIN_SPINE_OBJ_I_H
#define PLUGIN_SPINE_OBJ_I_H
#include "render/model/ui_object_ref.h"
#include "spine/Atlas.h"
#include "spine/SkeletonData.h"
#include "spine/Skeleton.h"
#include "spine/AnimationState.h"
#include "plugin/spine/plugin_spine_obj.h"
#include "plugin_spine_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_spine_obj {
    plugin_spine_module_t m_module;
    
    spSkeleton* m_skeleton;
    spBone* m_root_bone;

    spAnimationStateData* m_anim_state_data;
	spAnimationState* m_anim_state;

    uint8_t m_need_update;
	uint8_t m_debug_slots;
	uint8_t m_debug_bones;

    plugin_spine_obj_part_list_t m_parts;
    plugin_spine_obj_track_list_t m_tracks;
    plugin_spine_obj_ik_list_t m_iks;
    
    plugin_spine_track_listener_t m_listeners;
};

int plugin_spine_obj_do_init(void * ctx, ui_runtime_render_obj_t obj);
int plugin_spine_obj_do_set(void * ctx, ui_runtime_render_obj_t obj, UI_OBJECT_URL const * obj_url);    
int plugin_spine_obj_do_setup(void * ctx, ui_runtime_render_obj_t obj, char * args);    
void plugin_spine_obj_do_free(void * ctx, ui_runtime_render_obj_t obj);

int plugin_spine_obj_do_render(
    void * ctx, ui_runtime_render_obj_t obj,
    ui_runtime_render_t context, ui_rect_t clip_rect,
    ui_runtime_render_second_color_t second_color, ui_transform_t transform);

void plugin_spine_obj_do_update(void * ctx, ui_runtime_render_obj_t obj, float delta);

uint8_t plugin_spine_obj_do_is_playing(void * ctx, ui_runtime_render_obj_t obj);

void plugin_spine_obj_dispatch_cache_events(plugin_spine_obj_t obj);

void plugin_spine_obj_animation_callback(spAnimationState* state, int trackIndex, spEventType type, spEvent* event, int loopCount);
    
#ifdef __cplusplus
}
#endif

#endif
