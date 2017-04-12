#include <assert.h>
#include "spine/extension.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/random.h"
#include "render/model/ui_data_src.h"
#include "plugin/spine/plugin_spine_data_skeleton.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "plugin_spine_track_listener_i.h"
#include "plugin_spine_obj_i.h"
#include "plugin_spine_obj_part_i.h"
#include "plugin_spine_obj_track_i.h"
#include "plugin_spine_obj_anim_i.h"
#include "plugin_spine_obj_ik_i.h"

static void plugin_spine_obj_clear(plugin_spine_obj_t obj);
static void plugin_spine_obj_spine_event(void * ctx, plugin_spine_obj_anim_t anim, plugin_spine_anim_event_type_t type, spEvent* event);

int plugin_spine_obj_do_init(void * ctx, ui_runtime_render_obj_t render_obj) {
    plugin_spine_module_t module = ctx;
    plugin_spine_obj_t obj = (plugin_spine_obj_t)ui_runtime_render_obj_data(render_obj);

    bzero(obj, sizeof(*obj));

    obj->m_module = module;

    obj->m_skeleton = NULL;
    obj->m_anim_state_data = NULL;
    obj->m_anim_state = NULL;
    obj->m_need_update = 0;
    obj->m_debug_slots = 0;
	obj->m_debug_bones = 0;

    obj->m_listeners = NULL;
    TAILQ_INIT(&obj->m_tracks);
    TAILQ_INIT(&obj->m_parts);
    TAILQ_INIT(&obj->m_iks);

    plugin_spine_obj_add_listener(obj, plugin_spine_obj_spine_event, render_obj);

    return 0;
}

int plugin_spine_obj_do_set(void * ctx, ui_runtime_render_obj_t render_obj, UI_OBJECT_URL const * obj_url) {
    plugin_spine_module_t module = ctx;
    UI_OBJECT_URL_DATA_SKELETON const * skeleton_data = &obj_url->data.skeleton;
    plugin_spine_obj_t obj = (plugin_spine_obj_t)ui_runtime_render_obj_data(render_obj);
    ui_data_src_t spine_data_src;
    plugin_spine_data_skeleton_t spine_data;
    
    spine_data_src = ui_runtime_module_find_src(module->m_runtime, &skeleton_data->src, ui_data_src_type_spine_skeleton);
    if (spine_data_src == NULL) return -1;
    
    spine_data = (plugin_spine_data_skeleton_t)ui_data_src_product(spine_data_src);
    if (spine_data == NULL) {
        CPE_ERROR(module->m_em, "create plugin_spine_obj: skeleton data not loaded!");
        return -1;
    }

    if (plugin_spine_obj_set_data(obj, spine_data) != 0) return -1;

    if (skeleton_data->anim_def[0]) {
        if (plugin_spine_obj_play_anims(obj, skeleton_data->anim_def, NULL) != 0) {
            CPE_ERROR(module->m_em, "create plugin_spine_obj: start anim %s fail!", skeleton_data->anim_def);
            plugin_spine_obj_clear(obj);
            return -1;
        }
    }

    ui_runtime_render_obj_set_src(render_obj, spine_data_src);

    return 0;
}

int plugin_spine_obj_set_data(plugin_spine_obj_t obj, plugin_spine_data_skeleton_t spine_data) {
    plugin_spine_module_t module = obj->m_module;
    int ik_pos;
    
    plugin_spine_obj_clear(obj);
    
    obj->m_skeleton = spSkeleton_create(plugin_spine_data_skeleton_data(spine_data));
    if (obj->m_skeleton == NULL) {
        CPE_ERROR(module->m_em, "plugin_spine_obj_set_data: create skeleton fail!");
        return -1;
    }

    obj->m_anim_state_data = spAnimationStateData_create(obj->m_skeleton->data);
    if (obj->m_anim_state_data == NULL) {
        CPE_ERROR(module->m_em, "plugin_spine_obj_set_data: create anim state data fail!");
        plugin_spine_obj_clear(obj);
        return -1;
    }
    
	obj->m_anim_state = spAnimationState_create(obj->m_anim_state_data);
    if (obj->m_anim_state == NULL) {
        CPE_ERROR(module->m_em, "plugin_spine_obj_set_data: create anim state fail!");
        plugin_spine_obj_clear(obj);
        return -1;
    }

	obj->m_anim_state->rendererObject = obj;
    ((_spAnimationState*)obj->m_anim_state)->disposeTrackEntry = plugin_spine_obj_anim_dispose;

    for(ik_pos = 0; ik_pos < obj->m_skeleton->ikConstraintsCount; ++ik_pos) {
        plugin_spine_obj_ik_t ik = plugin_spine_obj_ik_create(obj, obj->m_skeleton->ikConstraints[ik_pos]);
        if (ik == NULL) {
            CPE_ERROR(module->m_em, "plugin_spine_obj_set_data: create ik fail!");
            plugin_spine_obj_clear(obj);
            return -1;
        }
    }
    
    return 0;
}

int plugin_spine_obj_do_setup(void * ctx, ui_runtime_render_obj_t render_obj, char * args) {
    plugin_spine_obj_t obj = (plugin_spine_obj_t)ui_runtime_render_obj_data(render_obj);
    ui_data_src_t src = ui_runtime_render_obj_src(render_obj);
    plugin_spine_module_t module = ctx;
    char * str_value;
    int rv = 0;

    if ((str_value = cpe_str_read_and_remove_arg(args, "debug-slots", ',', '='))) {
        obj->m_debug_slots = atoi(str_value);
    }

    if ((str_value = cpe_str_read_and_remove_arg(args, "debug-bones", ',', '='))) {
        obj->m_debug_slots = atoi(str_value);
    }

    if ((str_value = cpe_str_read_and_remove_arg(args, "skin", ',', '='))) {
        if (!spSkeleton_setSkinByName (obj->m_skeleton, str_value)) {
            CPE_ERROR(
                module->m_em, "spne obj %s(%s): skin %s not exist!",
                ui_runtime_render_obj_name(render_obj),
                src ? ui_data_src_data(src) : "",
                str_value);
            rv = -1;
        }
    }

    if ((str_value = cpe_str_read_and_remove_arg(args, "state", ',', '='))) {
        ui_data_src_t state_src = NULL;
        
        if (strcmp(str_value, "auto") == 0) {
            if (plugin_spine_obj_scane_parts(obj) != 0) {
                CPE_ERROR(
                    module->m_em, "spne obj %s(%s): auto scan state fail!",
                    ui_runtime_render_obj_name(render_obj), src ? ui_data_src_data(src) : "");
                rv = -1;
            }
        }
        else if (str_value[0]) {
            state_src = ui_data_src_find_by_path(module->m_data_mgr, str_value, ui_data_src_type_spine_state_def);
            if (state_src == NULL) {
                CPE_ERROR(
                    module->m_em, "spne obj %s(%s): state %s not exist!",
                    ui_runtime_render_obj_name(render_obj),
                    src ? ui_data_src_data(src) : "",
                    str_value);
                rv = -1;
            }
        }
        else {
            if (src == NULL) {
                CPE_ERROR(
                    module->m_em, "spne obj %s(): no src to detect state file!",
                    ui_runtime_render_obj_name(render_obj));
                rv = -1;
            }
            else {
                state_src = ui_data_src_child_find_by_path(ui_data_src_parent(src), ui_data_src_data(src), ui_data_src_type_spine_state_def);
                if (state_src == NULL) {
                    CPE_ERROR(
                        module->m_em, "spne obj %s(%s): state %s not exist!",
                        ui_runtime_render_obj_name(render_obj),
                        ui_data_src_data(src),
                        ui_data_src_data(src));
                    rv = -1;
                }
            }
        }

        if (state_src) {
            plugin_spine_data_state_def_t state_def;

            state_def = ui_data_src_product(state_src);
            if (state_def == NULL) {
                CPE_ERROR(
                    module->m_em, "spne obj %s(%s): state %s not loaded!",
                    ui_runtime_render_obj_name(render_obj),
                    src ? ui_data_src_data(src) : "", ui_data_src_data(state_src));
                rv = -1;
            }
            else if (plugin_spine_obj_build_parts(obj, state_def) != 0) {
                CPE_ERROR(
                    module->m_em, "spne obj %s(%s): build pargs from %s fail!",
                    ui_runtime_render_obj_name(render_obj),
                    src ? ui_data_src_data(src) : "", ui_data_src_data(state_src));
                rv = -1;
            }
        }
    }

    while ((str_value = cpe_str_read_and_remove_arg(args, "part-state", ',', '='))) {
        plugin_spine_obj_part_t part;
        char * sep = strchr(str_value, '~');
            
        if (sep == NULL) {
            CPE_ERROR(
                module->m_em, "spne obj %s(%s):  part-state %s format error!",
                ui_runtime_render_obj_name(render_obj),
                src ? ui_data_src_data(src) : "", str_value);
            rv = -1;
            continue;
        }

        *sep = 0;

        part = plugin_spine_obj_part_find(obj, str_value);
        if (part == NULL) {
            CPE_ERROR(
                module->m_em, "spne obj %s(%s):  part %s not exist!",
                ui_runtime_render_obj_name(render_obj),
                src ? ui_data_src_data(src) : "", str_value);
            rv = -1;
            continue;
        }

        if (*(sep + 1) != 0) {
            if (plugin_spine_obj_part_apply_transition_force(part, sep + 1) != 0) {
                CPE_ERROR(
                    module->m_em, "spne obj %s(%s):  part %s set to state %s fail!",
                    ui_runtime_render_obj_name(render_obj),
                    src ? ui_data_src_data(src) : "", str_value, sep + 1);
                rv = -1;
                continue;
            }
        }
        else {
            uint32_t state_count = plugin_spine_obj_part_state_count(part);
            if (state_count > 0) {
                struct plugin_spine_obj_part_state_it it;
                int32_t idx;
                plugin_spine_obj_part_state_t state;

                if (part->m_cur_state) state_count--;

                if (state_count > 0) {
                    idx = (int32_t)cpe_rand_dft(state_count);
                    plugin_spine_obj_part_states(part, &it);

                    while(idx >= 0) {
                        state = plugin_spine_obj_part_state_it_next(&it);
                        assert(state);
                        if (state != part->m_cur_state) --idx;
                    }
                    assert(state);

                    if (plugin_spine_obj_part_set_cur_state(part, state) != 0) {
                        CPE_ERROR(
                            module->m_em, "spne obj %s(%s):  part %s rand to state %s fail!",
                            ui_runtime_render_obj_name(render_obj),
                            src ? ui_data_src_data(src) : "", str_value, sep + 1);
                        rv = -1;
                        continue;
                    }
                }
            }
        }
    }
    
    return rv;
}

void plugin_spine_obj_clear(plugin_spine_obj_t obj) {
    while(!TAILQ_EMPTY(&obj->m_iks)) {
        plugin_spine_obj_ik_free(TAILQ_FIRST(&obj->m_iks));
    }
    
    while(!TAILQ_EMPTY(&obj->m_parts)) {
        plugin_spine_obj_part_free(TAILQ_FIRST(&obj->m_parts));
    }

    while(!TAILQ_EMPTY(&obj->m_tracks)) {
        plugin_spine_obj_track_free(TAILQ_FIRST(&obj->m_tracks));
    }

    if (obj->m_anim_state) {
        spAnimationState_dispose(obj->m_anim_state);
        obj->m_anim_state = NULL;
    }

    if (obj->m_anim_state_data) {
        spAnimationStateData_dispose(obj->m_anim_state_data);
        obj->m_anim_state_data = NULL;
    }
    
    if (obj->m_skeleton) {
        spSkeleton_dispose(obj->m_skeleton);
        obj->m_skeleton = NULL;
    }
}

void plugin_spine_obj_do_free(void * ctx, ui_runtime_render_obj_t render_obj) {
    plugin_spine_obj_t obj = (plugin_spine_obj_t)ui_runtime_render_obj_data(render_obj);
    plugin_spine_module_t module = ctx;

    plugin_spine_obj_clear(obj);

    plugin_spine_track_listener_free_list(module, &obj->m_listeners);
    assert(obj->m_listeners == NULL);
}

plugin_spine_module_t  plugin_spine_obj_module(plugin_spine_obj_t obj) {
    return obj->m_module;
}

uint8_t plugin_spine_obj_debug_slots(plugin_spine_obj_t obj) {
    return obj->m_debug_slots;
}

void plugin_spine_obj_set_debug_slots(plugin_spine_obj_t obj, uint8_t debug_slots) {
    obj->m_debug_slots = debug_slots;
}

uint8_t plugin_spine_obj_debug_bones(plugin_spine_obj_t obj) {
    return obj->m_debug_bones;
}

void plugin_spine_obj_set_debug_bones(plugin_spine_obj_t obj, uint8_t debug_bones) {
    obj->m_debug_bones = debug_bones;
}

void plugin_spine_obj_set_mix(plugin_spine_obj_t obj, const char * from_animation, const char * to_animation, float duration) {
	spAnimationStateData_setMixByName(obj->m_anim_state->data, from_animation, to_animation, duration);
}

void plugin_spine_obj_clear_tracks(plugin_spine_obj_t obj) {
	spAnimationState_clearTracks(obj->m_anim_state);
}

void plugin_spine_obj_clear_track(plugin_spine_obj_t obj, int track_index) {
	spAnimationState_clearTrack(obj->m_anim_state, track_index);
}

void plugin_spine_obj_do_update(void * ctx, ui_runtime_render_obj_t render_obj, float delta) {
    plugin_spine_obj_t obj = (plugin_spine_obj_t)ui_runtime_render_obj_data(render_obj);
    plugin_spine_obj_ik_t ik;
    
    if (obj->m_skeleton) {
        TAILQ_FOREACH(ik, &obj->m_iks, m_next) {
            if (ik->m_duration > 0.0f) plugin_spine_obj_ik_update(ik, delta);
        }
        
        spSkeleton_update(obj->m_skeleton, delta);
        spAnimationState_update(obj->m_anim_state, delta);
        spAnimationState_apply(obj->m_anim_state, obj->m_skeleton);
        spSkeleton_updateWorldTransform(obj->m_skeleton);
    }

    obj->m_need_update = 0;
}

uint8_t plugin_spine_obj_do_is_playing(void * ctx, ui_runtime_render_obj_t render_obj) {
    plugin_spine_obj_t obj = (plugin_spine_obj_t)ui_runtime_render_obj_data(render_obj);
    plugin_spine_obj_track_t track;

    TAILQ_FOREACH(track, &obj->m_tracks, m_next) {
        if (!TAILQ_EMPTY(&track->m_anims)) {
            return 1;
        }
    }
    
    return 0;
}

int plugin_spine_obj_set_ik_by_name(plugin_spine_obj_t obj, const char * ik_name, float x, float y) {
    uint16_t i;
    uint16_t processed = 0;
    
    for(i = 0; i < obj->m_skeleton->ikConstraintsCount; i++) {
        spIkConstraint* constraint = obj->m_skeleton->ikConstraints[i];
        if (strcmp(constraint->data->name, ik_name) == 0) {
            constraint->target->x = x;
            constraint->target->y = y;
            processed++;
        }
    }

    return processed > 0 ? 0 : -1;
}

static void plugin_spine_obj_spine_event(void * ctx, plugin_spine_obj_anim_t anim, plugin_spine_anim_event_type_t type, spEvent* event) {
    if (type == plugin_spine_anim_event_event) {
        assert(event);
        if (event->stringValue == NULL || event->stringValue[0] == 0) {
            ui_runtime_render_obj_send_event(ctx, event->data->name);
        }
        else {
            char event_buf[128];

            if (event->stringValue[0] == '[') {
                size_t len;
                const char * e = strchr(event->stringValue + 1, ']');
                if (e == NULL) {
                    ui_runtime_render_obj_t render_obj = (ui_runtime_render_obj_t)ctx;
                    plugin_spine_obj_t obj = (plugin_spine_obj_t)ui_runtime_render_obj_data(render_obj);
                    ui_data_src_t src = ui_runtime_render_obj_src(render_obj);
                    CPE_ERROR(
                        obj->m_module->m_em, "spine obj %s(%s): event %s format error",
                        ui_runtime_render_obj_name(render_obj),
                        src ? ui_data_src_data(src) : "", event->stringValue);
                    return;
                }

                len = (e - event->stringValue) + 1;

                if (len >= sizeof(event_buf)) {
                    ui_runtime_render_obj_t render_obj = (ui_runtime_render_obj_t)ctx;
                    plugin_spine_obj_t obj = (plugin_spine_obj_t)ui_runtime_render_obj_data(render_obj);
                    ui_data_src_t src = ui_runtime_render_obj_src(render_obj);
                    CPE_ERROR(
                        obj->m_module->m_em, "spine obj %s(%s): event %s len overflow!",
                        ui_runtime_render_obj_name(render_obj),
                        src ? ui_data_src_data(src) : "", event->stringValue);
                    return;
                }

                memcpy(event_buf, event->stringValue, len);
                snprintf(event_buf + len, sizeof(event_buf) - len, "%s: %s", event->data->name, e + 1);
            }
            else {
                snprintf(event_buf, sizeof(event_buf), "%s: %s", event->data->name, event->stringValue);
            }

            ui_runtime_render_obj_send_event(ctx, event_buf);
        }
    }
}

struct spSkeleton * plugin_spine_obj_skeleton(plugin_spine_obj_t obj) {
    return obj->m_skeleton;
}

struct spBone* plugin_spine_obj_find_bone_by_name(plugin_spine_obj_t obj, const char * name) {
    uint16_t bone_pos;
    for(bone_pos = 0; bone_pos < obj->m_skeleton->bonesCount; ++bone_pos) {
        struct spBone* bone = obj->m_skeleton->bones[bone_pos];
        if (strcmp(bone->data->name, name) == 0) return bone;
    }

    return NULL;
}

int plugin_spine_obj_apply_anim(plugin_spine_obj_t obj, const char * anim_name) {
    plugin_spine_module_t module = obj->m_module;
	spAnimation* animation = spSkeletonData_findAnimation(obj->m_skeleton->data, anim_name);
    int events_buf_count = 0;

    if (animation == NULL) {
        CPE_ERROR(module->m_em, "plugin_spine_obj_apply_anim: anim %s not exist!", anim_name);
        return -1;
    }

    spAnimation_apply(
        animation, obj->m_skeleton, -1.0f, animation->duration, 0,
        module->m_events_buf, &events_buf_count);
    
    return 0;
}
