#ifndef PLUGIN_SPINE_TYPES_H
#define PLUGIN_SPINE_TYPES_H
#include "cpe/utils/error.h"
#include "cpe/pal/pal_types.h"
#include "cpe/dr/dr_types.h"
#include "render/runtime/ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_spine_module * plugin_spine_module_t;
typedef struct plugin_spine_obj * plugin_spine_obj_t;
typedef struct plugin_spine_obj_part * plugin_spine_obj_part_t;
typedef struct plugin_spine_obj_part_state * plugin_spine_obj_part_state_t;
typedef struct plugin_spine_obj_part_state_it * plugin_spine_obj_part_state_it_t;
typedef struct plugin_spine_obj_part_transition * plugin_spine_obj_part_transition_t;
typedef struct plugin_spine_obj_track * plugin_spine_obj_track_t;
typedef struct plugin_spine_obj_anim * plugin_spine_obj_anim_t;
typedef struct plugin_spine_obj_anim_group * plugin_spine_obj_anim_group_t;
typedef struct plugin_spine_obj_ik * plugin_spine_obj_ik_t;
typedef struct plugin_spine_obj_ik_it * plugin_spine_obj_ik_it_t;

typedef struct plugin_spine_data_skeleton * plugin_spine_data_skeleton_t;
typedef struct plugin_spine_data_state_def * plugin_spine_data_state_def_t;
typedef struct plugin_spine_data_part * plugin_spine_data_part_t;
typedef struct plugin_spine_data_part_state * plugin_spine_data_part_state_t;
typedef struct plugin_spine_data_part_transition * plugin_spine_data_part_transition_t;

struct spSlot;    
struct spEvent;
struct spSkeleton;
struct spAtlas;
struct spBone;
struct spSkin;
struct spAttachment;
struct spIkConstraint;
typedef void * plugin_spine_data_attachment_t;
typedef struct plugin_spine_attachment_it * plugin_spine_attachment_it_t;

typedef enum plugin_spine_anim_event_type {
    plugin_spine_anim_event_start,
    plugin_spine_anim_event_end,
    plugin_spine_anim_event_complete,
    plugin_spine_anim_event_event,
    plugin_spine_anim_event_loop,
} plugin_spine_anim_event_type_t;
    
typedef void (*plugin_spine_anim_event_fun_t)(void * ctx, plugin_spine_obj_anim_t anim, plugin_spine_anim_event_type_t type, struct spEvent* event);

#ifdef __cplusplus
}
#endif

#endif
